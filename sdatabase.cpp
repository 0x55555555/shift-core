#include "sdatabase.h"
#include "sentity.h"
#include "schange.h"
#include "QFile"
#include "QRegExp"
#include "QDebug"
#include "styperegistry.h"
#include "Serialisation/sjsonio.h"
#include "xqtwrappers.h"
#include "spropertyinformationhelpers.h"
#include "XConvertScriptSTL.h"
#include "spropertycontaineriterators.h"

#ifdef X_DEBUG
# include "XMemoryTracker"
#endif

S_IMPLEMENT_PROPERTY(SDatabase, Shift)

void SDatabase::createTypeInformation(SPropertyInformationTyped<SDatabase> *info,
                                      const SPropertyInformationCreateData &data)
  {
  if(data.registerAttributes)
    {
    info->add(&SDatabase::majorVersion, "majorVersion")->setDefault(0);
    info->add(&SDatabase::minorVersion, "minorVersion")->setDefault(0);
    info->add(&SDatabase::revision, "revision")->setDefault(0);
    }

  if(data.registerInterfaces)
    {
    info->addInheritedInterface<SHandler>();

    auto *api = info->apiInterface();

    api->addMethod<QVector<SProperty *> (const QString &, QIODevice *, SPropertyContainer *), &SDatabase::load>("load");
    api->addMethod<void (const QString &, QIODevice *, SEntity *, bool, bool), &SDatabase::save>("save");
    }
  }

SDatabase::SDatabase()
  {
  _memory = STypeRegistry::allocator();
  xAssert(_memory);

#ifdef X_DEBUG
  XAllocatorBase* oldAllocator = _memory;
  _memory = new XMemoryTracker(oldAllocator);
#endif
  xAssert(_memory);

#ifdef S_CENTRAL_CHANGE_HANDLER
  _handler = this;
#else
  _stateStorageEnabled = false;
  SPropertyContainer::_database = this;
#endif
  setDatabase(this);
  _instanceInfo = &_instanceInfoData;
  }

SDatabase::~SDatabase()
  {
  uninitiatePropertyFromMetaData(this, typeInformation());
  _dynamicChild = 0;

  clearChanges();

#ifdef X_DEBUG
  XMemoryTracker* tracker = dynamic_cast<XMemoryTracker*>(_memory);
  xAssert(tracker);

  if(!tracker->empty())
    {
    //_memory.debugDump();
    xAssertFail();
    }
  delete _memory;
  _memory = 0;
#endif
  }

QVector<SProperty *> SDatabase::load(const QString &type, QIODevice *device, SPropertyContainer *loadRoot)
  {
  xAssert(type == "json");
  (void)type;

  SJSONLoader s;

  SProperty *p = loadRoot->lastChild();

  s.readFromDevice(device, loadRoot);

  if(!p)
    {
    p = loadRoot->firstChild();
    }

  QVector<SProperty *> ret;
  xForeach(auto c, loadRoot->walkerFrom(p))
    {
    ret << c;
    }

  return ret;
  }

void SDatabase::save(const QString &type, QIODevice *device, SEntity *saveRoot, bool readable, bool includeRoot)
  {
  xAssert(type == "json");
  (void)type;

  SJSONSaver s;
  s.setAutoWhitespace(readable);

  s.writeToDevice(device, saveRoot, includeRoot);
  }

SProperty *SDatabase::createDynamicProperty(const SPropertyInformation *type, SPropertyContainer *parentToBe, SPropertyInstanceInformationInitialiser *init)
  {
  SProfileFunction
  xAssert(type);

  void *propMem = _memory->alloc(type->dynamicSize());

  // new the prop type
  SProperty *prop = type->functions().createProperty(propMem);

  // new the instance information
  xuint8 *alignedPtr = (xuint8*)(propMem) + type->size();
  alignedPtr = X_ROUND_TO_ALIGNMENT(xuint8 *, alignedPtr);
  xAssertIsAligned(alignedPtr);

  SPropertyInstanceInformation *instanceInfoMem = (SPropertyInstanceInformation *)(alignedPtr);
  SPropertyInstanceInformation *instanceInfo = type->functions().createInstanceInformation(instanceInfoMem);

  instanceInfo->setDynamic(true);
  prop->_instanceInfo = instanceInfo;


  if(init)
    {
    init->initialise((SPropertyInstanceInformation*)prop->_instanceInfo);
    }

  instanceInfo->setChildInformation(type);


#ifdef S_CENTRAL_CHANGE_HANDLER
  prop->_handler = SHandler::findHandler(parentToBe, prop);
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

void SDatabase::deleteProperty(SProperty *prop)
{
  const SPropertyInstanceInformation *inst = prop->instanceInformation();
  const SPropertyInformation *info = inst->childInformation();

  xAssert(!prop->_flags.hasFlag(PreGetting));
  uninitiateProperty(prop);

  if(inst->extra())
    {
    info->functions().destroyProperty(prop);
    }
  }

void SDatabase::deleteDynamicProperty(SProperty *prop)
  {
  X_HEAP_CHECK
  deleteProperty(prop);
  X_HEAP_CHECK
  _memory->free(prop);
  X_HEAP_CHECK
  }

void SDatabase::initiateInheritedDatabaseType(const SPropertyInformation *info)
  {
  _instanceInfoData.setChildInformation(info);
  _instanceInfoData.setDynamic(true);
  initiatePropertyFromMetaData(this, info);
  }

void SDatabase::initiatePropertyFromMetaData(SPropertyContainer *container, const SPropertyInformation *mD)
  {
  xAssert(mD);

  xForeach(auto child, mD->childWalker())
    {
    // no contained properties with duplicated names...
    const SPropertyInformation *childInformation = child->childInformation();

    // extract the properties location from the meta data.
    SProperty *thisProp = child->locateProperty(container);

    if(child->extra())
      {
      childInformation->functions().createPropertyInPlace(thisProp);
      }

    thisProp->_instanceInfo = child;
    container->internalSetupProperty(thisProp);
    initiateProperty(thisProp);
    }
  }

void SDatabase::uninitiatePropertyFromMetaData(SPropertyContainer *container, const SPropertyInformation *mD)
  {
  xAssert(mD);

  xForeach(auto child, mD->childWalker())
    {
    // extract the properties location from the meta data.
    SProperty *thisProp = child->locateProperty(container);

    uninitiateProperty(thisProp);

    if(child->extra())
      {
      const SPropertyInformation *info = thisProp->typeInformation();
      info->functions().destroyProperty(thisProp);
      }
    }

  container->internalClear(this);
  xAssert(container->_dynamicChild == 0);
  }

void SDatabase::initiateProperty(SProperty *prop)
  {
  prop->typeInformation()->reference();

  SPropertyContainer *container = prop->castTo<SPropertyContainer>();
  if(container)
    {
    const SPropertyInformation *metaData = container->typeInformation();
    xAssert(metaData);

#ifndef S_CENTRAL_CHANGE_HANDLER
    container->_database = SPropertyContainer::_database;
#endif

    initiatePropertyFromMetaData(container, metaData);
    }


  if(prop->instanceInformation()->name() == "width")
  {
    xAssert(prop->isDirty());
  }
#ifdef S_CENTRAL_CHANGE_HANDLER
  xAssert(prop->handler());
#endif
  }

void SDatabase::postInitiateProperty(SProperty *prop)
  {
  SPropertyContainer *container = prop->castTo<SPropertyContainer>();
  if(container)
    {
    const SPropertyInformation *metaData = container->typeInformation();
    xAssert(metaData);

    xForeach(auto child, metaData->childWalker())
      {
      SProperty *thisProp = child->locateProperty(container);
      postInitiateProperty(thisProp);
      }
    }

  const SPropertyInstanceInformation *inst = prop->instanceInformation();
  xAssert(inst);
  inst->initiateProperty(prop);

#ifdef S_PROPERTY_POST_CREATE
  const SPropertyInformation *info = prop->typeInformation();
  while(info)
    {
    SPropertyInformation::PostCreateFunction postCreate = info->functions().postCreate;
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

void SDatabase::uninitiateProperty(SProperty *prop)
  {
  prop->typeInformation()->dereference();

  SPropertyContainer *container = prop->castTo<SPropertyContainer>();
  if(container)
    {
    const SPropertyInformation *metaData = container->typeInformation();
    xAssert(metaData);

    uninitiatePropertyFromMetaData(container, metaData);
    }
  }

QChar SDatabase::pathSeparator()
  {
  return QChar('/');
  }
