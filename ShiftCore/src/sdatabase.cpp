#include "shift/sdatabase.h"
#include "shift/sentity.h"
#include "shift/Changes/schange.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/Properties/spropertycontaineriterators.h"
#include "shift/Serialisation/sjsonio.h"
#include "xqtwrappers.h"
#include "XConvertScriptSTL.h"
#include "QFile"
#include "QRegExp"
#include "QDebug"

#ifdef X_DEBUG
# include "XMemoryTracker"
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
      api->method<QVector<Property *> (const QString &, QIODevice *, PropertyContainer *), &Database::load>("load"),
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
  _memory = TypeRegistry::generalPurposeAllocator()->create<Eks::MemoryTracker>(oldAllocator);
#endif
  xAssert(_memory);

  _temporaryMemory = TypeRegistry::temporaryAllocator();

#ifdef S_CENTRAL_CHANGE_HANDLER
  _handler = this;
#else
  PropertyContainer::_database = this;
#endif
  setDatabase(this);
  _instanceInfo = &_instanceInfoData;
  }

Database::~Database()
  {
  uninitiatePropertyFromMetaData(this, typeInformation());
  _dynamicChild = 0;

  clearChanges();

#ifdef X_DEBUG
  Eks::MemoryTracker* tracker = dynamic_cast<Eks::MemoryTracker*>(_memory);
  xAssert(tracker);

  if(!tracker->empty())
    {
    //_memory.debugDump();
    xAssertFail();
    }
  TypeRegistry::generalPurposeAllocator()->destroy(_memory);
  _memory = 0;
#endif
  }

QVector<Property *> Database::load(const QString &type, QIODevice *device, PropertyContainer *loadRoot)
  {
  xAssert(type == "json");
  (void)type;

  SJSONLoader s;

  Property *p = loadRoot->lastChild();

  s.readFromDevice(device, loadRoot);

  if(!p)
    {
    p = loadRoot->firstChild();
    }

  QVector<Property *> ret;
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

Property *Database::createDynamicProperty(const PropertyInformation *type, PropertyContainer *parentToBe, PropertyInstanceInformationInitialiser *init)
  {
  SProfileFunction
  xAssert(type);

  void *propMem = _memory->alloc(type->dynamicSize());

  // new the prop type
  Property *prop = type->functions().createProperty(propMem);

  // new the instance information
  xuint8 *alignedPtr = (xuint8*)(propMem) + type->size();
  alignedPtr = Eks::roundToAlignment(alignedPtr);
  xAssertIsAligned(alignedPtr);
  PropertyInstanceInformation *instanceInfoMem = (PropertyInstanceInformation *)(alignedPtr);
  PropertyInstanceInformation *instanceInfo = type->functions().createDynamicInstanceInformation(instanceInfoMem, 0);

  prop->_instanceInfo = instanceInfo;

  if(init)
    {
    init->initialise((PropertyInstanceInformation*)prop->_instanceInfo);
    }

  instanceInfo->setChildInformation(type);


#ifdef S_CENTRAL_CHANGE_HANDLER
  prop->_handler = Handler::findHandler(parentToBe, prop);
  xAssert(_handler);
#else
  (void)parentToBe;
#endif

  initiateProperty(prop);
  xAssert(prop->isDirty());
  postInitiateProperty(prop);
  xAssert(prop->isDirty());
  return prop;
  }

void Database::deleteProperty(Property *prop)
  {
  xAssert(!prop->_flags.hasFlag(PreGetting));
  uninitiateProperty(prop);

  if(!prop->isDynamic())
    {
    const EmbeddedPropertyInstanceInformation *inst = prop->embeddedBaseInstanceInformation();
    if(inst->isExtraClassMember())
      {
      const PropertyInformation *info = inst->childInformation();
      info->functions().destroyProperty(prop);
      }
    }
  }

void Database::deleteDynamicProperty(Property *prop)
  {
  X_HEAP_CHECK
  deleteProperty(prop);
  X_HEAP_CHECK
  _memory->free(prop);
  X_HEAP_CHECK
  }

void Database::initiateInheritedDatabaseType(const PropertyInformation *info)
  {
  _instanceInfoData.setChildInformation(info);
  initiatePropertyFromMetaData(this, info);

  postInitiateProperty(this);
  }

void Database::initiatePropertyFromMetaData(PropertyContainer *container, const PropertyInformation *mD)
  {
  xAssert(mD);

  xForeach(auto child, mD->childWalker())
    {
    // no contained properties with duplicated names...
    const PropertyInformation *childInformation = child->childInformation();

    // extract the properties location from the meta data.
    Property *thisProp = child->locateProperty(container);

    if(child->isExtraClassMember())
      {
      childInformation->functions().createPropertyInPlace(thisProp);
      }

    thisProp->_instanceInfo = child;
    container->internalSetupProperty(thisProp);
    initiateProperty(thisProp);
    }
  }

void Database::uninitiatePropertyFromMetaData(PropertyContainer *container, const PropertyInformation *mD)
  {
  xAssert(mD);

  xForeach(auto child, mD->childWalker())
    {
    // extract the properties location from the meta data.
    Property *thisProp = child->locateProperty(container);

    uninitiateProperty(thisProp);

    if(child->isExtraClassMember())
      {
      const PropertyInformation *info = thisProp->typeInformation();
      info->functions().destroyProperty(thisProp);
      }
    }

  container->internalClear(this);
  xAssert(container->_dynamicChild == 0);
  }

void Database::initiateProperty(Property *prop)
  {
  prop->typeInformation()->reference();

  PropertyContainer *container = prop->castTo<PropertyContainer>();
  if(container)
    {
    const PropertyInformation *metaData = container->typeInformation();
    xAssert(metaData);

#ifndef S_CENTRAL_CHANGE_HANDLER
    container->_database = PropertyContainer::_database;
#endif

    initiatePropertyFromMetaData(container, metaData);
    }

#ifdef S_CENTRAL_CHANGE_HANDLER
  xAssert(prop->handler());
#endif
  }

void Database::postInitiateProperty(Property *prop)
  {
  PropertyContainer *container = prop->castTo<PropertyContainer>();
  if(container)
    {
    const PropertyInformation *metaData = container->typeInformation();
    xAssert(metaData);

    xForeach(auto child, metaData->childWalker())
      {
      Property *thisProp = child->locateProperty(container);
      postInitiateProperty(thisProp);
      }
    }

  if(!prop->isDynamic())
    {
    const EmbeddedPropertyInstanceInformation *inst = prop->embeddedBaseInstanceInformation();
    xAssert(inst);
    inst->initiateProperty(prop);
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

#ifdef S_CENTRAL_CHANGE_HANDLER
  xAssert(prop->handler());
#endif
  }

void Database::uninitiateProperty(Property *prop)
  {
  prop->typeInformation()->dereference();

  PropertyContainer *container = prop->castTo<PropertyContainer>();
  if(container)
    {
    const PropertyInformation *metaData = container->typeInformation();
    xAssert(metaData);

    uninitiatePropertyFromMetaData(container, metaData);
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
