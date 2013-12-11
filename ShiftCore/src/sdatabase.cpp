#include "shift/sdatabase.h"
#include "shift/sentity.h"
#include "shift/Changes/schange.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/Properties/scontaineriterators.h"
#include "shift/Properties/scontainer.inl"
#include "shift/Serialisation/sjsonio.h"
#include "xqtwrappers.h"
#include "XConvertScriptSTL.h"
#include "QFile"
#include "QRegExp"
#include "QDebug"

#ifdef X_DEBUG
# include "XLoggingAllocator"
#endif

Q_DECLARE_METATYPE(Eks::Vector<Shift::Attribute *>)

#ifdef S_DEBUGGER
# include "shift/UI/sdebugger.h"
#endif

namespace Shift
{

class AttributeInitialiserHelperImpl : public AttributeInitialiserHelper
  {
public:
  AttributeInitialiserHelperImpl(Database *db) : alloc(db->temporaryAllocator()), _calls(&alloc)
    {
    }

  void treeComplete()
    {
    xForeach(const auto &c, _calls)
      {
      c.cb(c.data);
      }
    }

  void onTreeComplete(Callback cb, void *data) X_OVERRIDE
    {
    Call call = { cb, data };
    _calls.pushBack(call);
    }

  virtual Eks::AllocatorBase* allocator() X_OVERRIDE
    {
    return &alloc;
    }

private:
  Eks::TemporaryAllocator alloc;

  struct Call
    {
    Callback cb;
    void *data;
    };
  Eks::Vector<Call> _calls;
  };

S_IMPLEMENT_PROPERTY(Database, Shift)

void Database::createTypeInformation(PropertyInformationTyped<Database> *info,
                                      const PropertyInformationCreateData &data)
  {
  if(data.registerAttributes)
    {
    auto childBlock = info->createChildrenBlock(data);
    }
  if(data.registerInterfaces)
    {
    auto *api = info->apiInterface();

    static XScript::ClassDef<0,0,2> cls = {
      {
      api->method<Eks::Vector<Attribute *> (const QString &, QIODevice *, Container *), &Database::load>("load"),
      api->method<void (const QString &, QIODevice *, Entity *, bool, bool), &Database::save>("save"),
      }
    };

    api->buildInterface(cls);
    }
  }

Database::Database()
    : _editCaches(TypeRegistry::generalPurposeAllocator())
  {
  _memory = TypeRegistry::persistentBlockAllocator();
  xAssert(_memory);

#ifdef X_DEBUG
  Eks::AllocatorBase* oldAllocator = _memory;
  _memory = TypeRegistry::generalPurposeAllocator()->create<Eks::LoggingAllocator>(oldAllocator);
#endif
  xAssert(_memory);

#ifdef S_CENTRAL_CHANGE_HANDLER
  _handler = this;
#else
  Entity::_database = this;
#endif
  setDatabase(this);
  _instanceInfo = &_instanceInfoData;

#ifdef S_DEBUGGER
  if (QApplication::instance())
    {
    _debugger.create(Eks::Core::defaultAllocator());
    _debugger->show();
    }
#endif
  }

Database::~Database()
  {
  uninitiateAttributeFromMetaData(this, typeInformation());
  _dynamicChild = 0;

  clearChanges();

#ifdef X_DEBUG
  Eks::LoggingAllocator* tracker = dynamic_cast<Eks::LoggingAllocator*>(_memory);
  xAssert(tracker);

  TypeRegistry::generalPurposeAllocator()->destroy(_memory);
  _memory = 0;
#endif
  }

Eks::Vector<Attribute *> Database::load(const QString &type, QIODevice *device, Container *loadRoot)
  {
  xAssert(type == "json");
  (void)type;

  JSONLoader s;

  Attribute *p = loadRoot->lastChild();

  s.readFromDevice(device, loadRoot);

  if(!p)
    {
    p = loadRoot->firstChild();
    }

  Eks::Vector<Attribute *> ret;
  xForeach(auto c, loadRoot->walkerFrom(p))
    {
    ret << c;
    }

  return ret;
  }

void Database::save(const QString &type, QIODevice *device, Entity *saveRoot, bool readable, bool includeRoot)
  {
  xAssert(type == "json");
  (void)type;

  JSONSaver s;
  s.setAutoWhitespace(readable);

  s.writeToDevice(device, saveRoot, includeRoot);
  }

Attribute *Database::addDynamicAttribute(
    const PropertyInformation *type,
    const NameArg & name,
    xsize index,
    Container *parent,
    PropertyInstanceInformationInitialiser *init)
  {
  SProfileFunction
  xAssert(type);

  auto fmt = type->format() + type->dynamicInstanceInformationFormat();
  // bucket allocator requires this minimum alignemnt
  fmt.alignTo(X_ALIGN_BYTE_COUNT);

  Eks::MemoryResource propMem = _memory->alloc(fmt);
  Eks::MemoryResource instanceInfoMemUnaligned = propMem.increment(type->format().size());
  Eks::MemoryResource instanceInfoMem = instanceInfoMemUnaligned.align(type->dynamicInstanceInformationFormat().alignment());

  // new the prop type
  xAssert(type->functions().create);
  Attribute *prop = type->functions().create(propMem);

  // new the instance information
  PropertyInstanceInformation *instanceInfo =
    type->functions().createDynamicInstanceInformation(instanceInfoMem, 0);

  prop->_instanceInfo = instanceInfo;
  xAssert(prop->isDynamic());

  if(init)
    {
    init->initialise((PropertyInstanceInformation*)prop->_instanceInfo);
    }

  instanceInfo->setChildInformation(type);


#ifdef S_CENTRAL_CHANGE_HANDLER
  if (Entity *ent = prop->castTo<Entity>())
    {
    ent->_handler = Handler::findHandler(parent, ent);
    xAssert(_handler);
    xAssert(ent->_handler);
    }
#endif

  // insert the property into the tree, before running post initiate operations
  // this allows things like connections to be made in initiate attribute.
  parent->makeUniqueName(prop, name, ((PropertyInstanceInformation*)prop->_instanceInfo)->name());

  PropertyDoChange(ContainerTreeChange, (Container*)0, parent, prop, index);

  AttributeInitialiserHelperImpl helper(this);

  // We call this after adding it to the tree so flags like ParentHasInput are setup at the root.
  initiateAttribute(prop, &helper);

  helper.treeComplete();
  xAssert(!prop->castTo<Property>() || prop->uncheckedCastTo<Property>()->isDirty());

  return prop;
  }

void Database::deleteDynamicAttribute(Attribute *prop)
  {
  X_HEAP_CHECK

#if X_ASSERTS_ENABLED
  if(Property *p = prop->castTo<Property>())
    {
    xAssert(!p->_input);
    xAssert(!p->_output);
    xAssert(!p->_nextOutput);
    xAssert(p->isDynamic());

    xAssert(!p->isUpdating());
    }
#endif

  uninitiateAttribute(prop);

  const PropertyInformation *info = prop->typeInformation();

  prop->_instanceInfo = 0;
  Eks::MemoryResource mem = info->functions().destroy(prop);

  X_HEAP_CHECK

  _memory->free(mem);
  X_HEAP_CHECK
  }

Database::EditCache* Database::findEditCache(Container *c)
  {
  if(_lastEditCache.first == c)
    {
    return _lastEditCache.second;
    }

  EditCache* cache = _editCaches.value(c, nullptr);

  _lastEditCache.first = c;
  _lastEditCache.second = cache;
  return cache;
  }

void Database::addEditCache(Container *c, EditCache *e)
  {
  _editCaches[c] = e;

  _lastEditCache.first = c;
  _lastEditCache.second = e;
  }

void Database::removeEditCache(Container *c)
  {
  xAssert(_editCaches.contains(c));
  _editCaches.remove(c);

  if(_lastEditCache.first == c)
    {
    _lastEditCache.second = nullptr;
    }
  }

void Database::initiateInheritedDatabaseType(const PropertyInformation *info)
  {
  AttributeInitialiserHelperImpl helper(this);

  _instanceInfoData.setChildInformation(info);
  initiateAttributeFromMetaData(this, info, &helper);

  helper.treeComplete();
  }

void Database::initiateAttributeFromMetaData(
    Container *container,
    const PropertyInformation *mD,
    AttributeInitialiserHelper *helper)
  {
  xAssert(mD);

  xForeach(auto child, mD->childWalker())
    {
    // no contained properties with duplicated names...
    const PropertyInformation *childInformation = child->childInformation();

    // extract the properties location from the meta data.
    Attribute *thisProp = child->locate(container);

    if(child->isExtraClassMember())
      {
      Eks::MemoryResource thisAttrMem(thisProp);

      const xsize dataOffset = childInformation->propertyDataOffset();
      Eks::MemoryResource thisDerivedMem = thisAttrMem.decrememt(dataOffset);

      X_USED_FOR_ASSERTS(Attribute *created =)
        childInformation->functions().create(thisDerivedMem);
      xAssert(created == thisProp);
      }

    thisProp->_instanceInfo = child;
    container->internalSetup(thisProp);
    initiateAttribute(thisProp, helper);
    }
  }

void Database::uninitiateAttributeFromMetaData(Container *container, const PropertyInformation *mD)
  {
  xAssert(mD);

  xForeach(auto child, mD->childWalker())
    {
    // extract the properties location from the meta data.
    Attribute *thisProp = child->locate(container);

    uninitiateAttribute(thisProp);

    if(child->isExtraClassMember())
      {
      const PropertyInformation *info = thisProp->typeInformation();
      info->functions().destroy(thisProp);
      }
    }

  container->internalClear(this);
  xAssert(container->_dynamicChild == 0);
  }

void Database::initiateAttribute(Attribute *prop, AttributeInitialiserHelper* initialiser)
  {
  prop->typeInformation()->reference();

  Container *container = prop->castTo<Container>();
  if(container)
    {
    const PropertyInformation *metaData = container->typeInformation();
    xAssert(metaData);

#ifndef S_CENTRAL_CHANGE_HANDLER
    if(Entity *ent = container->castTo<Entity>())
      {
      ent->_database = Entity::_database;
      }
#endif

    initiateAttributeFromMetaData(container, metaData, initialiser);
    }

  if(!prop->isDynamic())
    {
    const EmbeddedPropertyInstanceInformation *inst = prop->embeddedBaseInstanceInformation();
    xAssert(inst);
    inst->initiateAttribute(prop, initialiser);
    }

#ifdef S_PROPERTY_POST_CREATE
  const PropertyInformation *info = prop->typeInformation();
  while(info)
    {
    PropertyInformation::PostCreateFunction postCreate = info->functions().postCreate;
    if(postCreate)
      {
      postCreate(prop);
      }
    info = info->parentTypeInformation();
    }
#endif

  xAssert(!prop->castTo<Property>() || prop->uncheckedCastTo<Property>()->isDirty());
  }

void Database::uninitiateAttribute(Attribute *prop)
  {
  prop->typeInformation()->dereference();

  Container *container = prop->castTo<Container>();
  if(container)
    {
    const PropertyInformation *metaData = container->typeInformation();
    xAssert(metaData);

    uninitiateAttributeFromMetaData(container, metaData);
    }

  const PropertyInstanceInformation* inst = prop->baseInstanceInformation();
  if(inst->isDynamic())
    {
    PropertyInformationFunctions::DestroyInstanceInformationFunction destroy =
      inst->childInformation()->functions().destroyDynamicInstanceInformation;

    PropertyInstanceInformation *instanceInfo = const_cast<PropertyInstanceInformation*>(inst);
    destroy(instanceInfo);
    }
  }

const Eks::Char *Database::pathSeparator()
  {
  return "/";
  }

const Eks::Char *Database::escapedPathSeparator()
  {
  return "\\/";
  }

}
