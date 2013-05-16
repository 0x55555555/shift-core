#include "shift/sdatabase.h"
#include "shift/sentity.h"
#include "shift/Changes/schange.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/Properties/scontaineriterators.h"
#include "shift/Serialisation/sjsonio.h"
#include "xqtwrappers.h"
#include "XConvertScriptSTL.h"
#include "QFile"
#include "QRegExp"
#include "QDebug"

#ifdef X_DEBUG
# include "XLoggingAllocator"
#endif

namespace Shift
{

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

  SJSONLoader s;

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

  SJSONSaver s;
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

  xsize size = type->dynamicSize();
  size = Eks::roundToAlignment(size, X_ALIGN_BYTE_COUNT);

  void *propMem = _memory->alloc(size);

  // new the prop type
  xAssert(type->functions().create);
  Attribute *prop = type->functions().create(propMem);

  // new the instance information
  xuint8 *alignedPtr = (xuint8*)(propMem) + type->size();
  const xsize alignment = type->dynamicInstanceInformationFormat().alignment();

  alignedPtr = Eks::roundToAlignment(alignedPtr, alignment);
  xAssertIsSpecificAligned(alignedPtr, alignment);

  PropertyInstanceInformation *instanceInfoMem = (PropertyInstanceInformation *)(alignedPtr);
  PropertyInstanceInformation *instanceInfo = type->functions().createDynamicInstanceInformation(instanceInfoMem, 0);

  prop->_instanceInfo = instanceInfo;

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
  bool nameUnique = !name.isEmpty() && internalFindChild(name) == false;
  if(!nameUnique)
    {
    makeUniqueName(prop, name, ((PropertyInstanceInformation*)prop->_instanceInfo)->name());
    }
  else
    {
    name.toName(((PropertyInstanceInformation*)prop->_instanceInfo)->name());
    }

  PropertyDoChange(TreeChange, (Container*)0, parent, prop, index);

  // We call this after adding it to the tree so flags like ParentHasInput are setup at the root.
  initiateAttribute(prop);
  xAssert(!prop->castTo<Property>() || prop->uncheckedCastTo<Property>()->isDirty());

  postInitiateAttribute(prop);
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
  void *mem = info->functions().destroy(prop);

  X_HEAP_CHECK

  _memory->free(mem);
  X_HEAP_CHECK
  }

void Database::initiateInheritedDatabaseType(const PropertyInformation *info)
  {
  _instanceInfoData.setChildInformation(info);
  initiateAttributeFromMetaData(this, info);

  postInitiateAttribute(this);
  }

void Database::initiateAttributeFromMetaData(Container *container, const PropertyInformation *mD)
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
      childInformation->functions().createInPlace(thisProp);
      }

    thisProp->_instanceInfo = child;
    container->internalSetup(thisProp);
    initiateAttribute(thisProp);
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

void Database::initiateAttribute(Attribute *prop)
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

    initiateAttributeFromMetaData(container, metaData);
    }
  }

void Database::postInitiateAttribute(Attribute *prop)
  {
  Container *container = prop->castTo<Container>();
  if(container)
    {
    const PropertyInformation *metaData = container->typeInformation();
    xAssert(metaData);

    xForeach(auto child, metaData->childWalker())
      {
      Attribute *thisProp = child->locate(container);
      postInitiateAttribute(thisProp);
      }
    }

  if(!prop->isDynamic())
    {
    const EmbeddedPropertyInstanceInformation *inst = prop->embeddedBaseInstanceInformation();
    xAssert(inst);
    inst->initiateAttribute(prop);
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
