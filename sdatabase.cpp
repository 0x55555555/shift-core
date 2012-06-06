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


  _handler = this;
  setDatabase(this);
  _info = staticTypeInformation();
  _instanceInfo = &_instanceInfoData;
  }

SDatabase::~SDatabase()
  {
  internalClear();
  _child = 0;

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

  SProperty *p = loadRoot->firstChild();
  while(p && p->nextSibling())
    {
    p = p->nextSibling();
    }

  s.readFromDevice(device, loadRoot);

  if(!p)
    {
    p = loadRoot->firstChild();
    }

  QVector<SProperty *> ret;
  SProperty *c = p;
  while(c)
    {
    ret << c;
    c = c->nextSibling();
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
  xuint8 *alignedPtr = (xuint8*)(prop) + type->size();
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

  prop->_info = type;
  prop->_handler = SHandler::findHandler(parentToBe, prop);
  xAssert(_handler);

  initiateProperty(prop);
  xAssert(prop->isDirty());
  postInitiateProperty(prop);
  xAssert(prop->isDirty());
  return prop;
  }

void SDatabase::deleteProperty(SProperty *prop)
  {
  const SPropertyInformation *info = prop->typeInformation();

  xAssert(!prop->_flags.hasFlag(PreGetting));
  uninitiateProperty(prop);

  info->functions().destroyProperty(prop);
  }

void SDatabase::deleteDynamicProperty(SProperty *prop)
  {
  deleteProperty(prop);
  _memory->free(prop);
  }

void SDatabase::initiateInheritedDatabaseType(const SPropertyInformation *info)
  {
  _info = info;
  initiatePropertyFromMetaData(this, info);
  }

void SDatabase::initiatePropertyFromMetaData(SPropertyContainer *container, const SPropertyInformation *mD)
  {
  xAssert(mD);

  for(xsize i=0, s=mD->childCount(); i<s; ++i)
    {
    // no contained properties with duplicated names...
    const SPropertyInstanceInformation *child = mD->childFromIndex(i);
    const SPropertyInformation *childInformation = child->childInformation();

    // extract the properties location from the meta data.
    SProperty *thisProp = child->locateProperty(container);

    if(child->extra())
      {
      childInformation->functions().createPropertyInPlace(thisProp);
      }

    xAssert(thisProp->_parent == 0);
    xAssert(thisProp->_entity == 0);
    xAssert(thisProp->_nextSibling == 0);

    thisProp->_info = childInformation;
    thisProp->_instanceInfo = child;
    container->internalInsertProperty(true, thisProp, X_SIZE_SENTINEL);
    initiateProperty(thisProp);
    }
  }

void SDatabase::uninitiatePropertyFromMetaData(SPropertyContainer *container, const SPropertyInformation *mD)
  {
  xAssert(mD);

  for(xsize i=0; i<mD->childCount(); ++i)
    {
    // no contained properties with duplicated names...
    const SPropertyInstanceInformation *child = mD->childFromIndex(i);

    // extract the properties location from the meta data.
    SProperty *thisProp = child->locateProperty(container);

    uninitiateProperty(thisProp);

    if(child->extra())
      {
      const SPropertyInformation *info = thisProp->typeInformation();
      info->functions().destroyProperty(thisProp);
      }
    }
  }

void SDatabase::initiateProperty(SProperty *prop)
  {
  prop->typeInformation()->reference();

  SPropertyContainer *container = prop->castTo<SPropertyContainer>();
  if(container)
    {
    const SPropertyInformation *metaData = container->typeInformation();
    xAssert(metaData);

    initiatePropertyFromMetaData(container, metaData);
    }


  if(prop->instanceInformation()->name() == "width")
  {
    xAssert(prop->isDirty());
  }
  xAssert(prop->handler());
  }

void SDatabase::postInitiateProperty(SProperty *prop)
  {
  if(prop->instanceInformation()->name() == "width")
  {
    xAssert(prop->isDirty());
  }
  SPropertyContainer *container = prop->castTo<SPropertyContainer>();
  if(container)
    {
    const SPropertyInformation *metaData = container->typeInformation();
    xAssert(metaData);

    for(xsize i=0, s=metaData->childCount(); i<s; ++i)
      {
      const SPropertyInstanceInformation *child = metaData->childFromIndex(i);

      SProperty *thisProp = child->locateProperty(container);
      postInitiateProperty(thisProp);
      }
    }

  const SPropertyInstanceInformation *inst = prop->instanceInformation();
  xAssert(inst);
  if(inst->name() == "width")
  {
    xAssert(prop->isDirty());
  }
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

  xAssert(prop->handler());
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
