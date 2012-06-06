#include "sentity.h"
#include "QStringList"
#include "styperegistry.h"
#include "sdatabase.h"
#include "sinterfaces.h"
#include "spropertyinformationhelpers.h"

S_IMPLEMENT_PROPERTY(SEntity, Shift)

void SEntity::createTypeInformation(SPropertyInformationTyped<SEntity> *info,
                                    const SPropertyInformationCreateData &data)
  {
  if(data.registerAttributes)
    {
    auto *childInst = info->add(&SEntity::children, "children");
    childInst->setMode(SPropertyInstanceInformation::Internal);
    }

  if(data.registerInterfaces)
    {
    auto *api = info->apiInterface();
    api->addMethod<SProperty* (const SPropertyInformation *, const QString &), &SEntity::addChild>("addChild");

    api->addMethod<void (STreeObserver* obs), &SEntity::addTreeObserver>("addTreeObserver");
    api->addMethod<void (SDirtyObserver* obs), &SEntity::addDirtyObserver>("addDirtyObserver");
    api->addMethod<void (SConnectionObserver* obs), &SEntity::addConnectionObserver>("addConnectionObserver");

    api->addMethod<void (STreeObserver* obs), &SEntity::removeTreeObserver>("removeTreeObserver");
    api->addMethod<void (SDirtyObserver* obs), &SEntity::removeDirtyObserver>("removeDirtyObserver");
    api->addMethod<void (SConnectionObserver* obs), &SEntity::removeConnectionObserver>("removeConnectionObserver");

    info->addStaticInterface(new SBasicPositionInterface);
    }
  }

template <typename T> void xRemoveAll(QVector<T>& vec, const T &ptr)
  {
  for(int i = 0; i < vec.size(); ++i)
    {
    if(vec[i] == ptr)
      {
      vec.remove(i--);
      }
    }
  }

SEntity::~SEntity()
  {
  for(int o = 0; o < _observers.size(); ++o)
    {
    SObserver* observer = _observers[o].getObserver();
    QVector<SEntity*>& ents = observer->_entities;
    xRemoveAll(ents, this);
    }
  _observers.clear();
  }

void SEntity::reparent(SEntity *ent)
  {
  xAssert(parent() && parentEntity());

  parent()->moveProperty(&ent->children, this);
  }

SProperty *SEntity::addChild(const SPropertyInformation *info, const QString& name)
  {
  SBlock b(handler());
  SProperty *ent = children.add(info, name);
  xAssert(ent);
  return ent;
  }

SProperty *SEntity::addProperty(const SPropertyInformation *info, const QString& name, SPropertyInstanceInformationInitialiser *inst)
  {
  SBlock b(handler());
  SProperty *p = SPropertyContainer::addProperty(info, X_SIZE_SENTINEL, name, inst);
  xAssert(p);
  return p;
  }

SEntity *SEntity::parentEntity() const
  {
  if(parent())
    {
    return parent()->entity();
    }
  return 0;
  }

SEntity *SEntity::findChildEntity(const QString &n)
  {
  SProperty *prop = children.findChild(n);
  if(!prop)
    {
    return 0;
    }
  return prop->castTo<SEntity>();
  }

const SEntity *SEntity::findChildEntity(const QString &n) const
  {
  const SProperty *prop = children.findChild(n);
  if(!prop)
    {
    return 0;
    }
  return prop->castTo<SEntity>();
  }

SEntity *SEntity::firstChildEntity() const
  {
  if(children.firstChild())
    {
    return children.firstChild()->entity();
    }
  return 0;
  }

SEntity *SEntity::nextSiblingEntity() const
  {
  if(nextSibling())
    {
    return nextSibling()->entity();
    }
  return 0;
  }

void SEntity::saveProperty(const SProperty *p, SSaver &l)
  {
  SPropertyContainer::saveProperty(p, l);
  }

SProperty *SEntity::loadProperty(SPropertyContainer *p, SLoader &l)
  {
  return SPropertyContainer::loadProperty(p, l);
  }

SObserver *SEntity::ObserverStruct::getObserver()
  {
  if(mode == ObserverStruct::Tree)
    {
    return (STreeObserver*)observer;
    }
  else if(mode == ObserverStruct::Dirty)
    {
    return (SDirtyObserver*)observer;
    }
  else if(mode == ObserverStruct::Connection)
    {
    return (SConnectionObserver*)observer;
    }
  return 0;
  }

void SEntity::addDirtyObserver(SDirtyObserver *in)
  {
  ObserverStruct s;
  s.mode = ObserverStruct::Dirty;
  s.observer = in;
  in->_entities << this;
  _observers << s;
  }

void SEntity::addTreeObserver(STreeObserver *in)
  {
  xAssert(in);
  ObserverStruct s;
  s.mode = ObserverStruct::Tree;
  s.observer = in;
  in->_entities << this;
  _observers << s;
  }

void SEntity::addConnectionObserver(SConnectionObserver *in)
  {
  ObserverStruct s;
  s.mode = ObserverStruct::Connection;
  s.observer = in;
  in->_entities << this;
  _observers << s;
  }

void SEntity::removeDirtyObserver(SDirtyObserver *in)
  {
  removeObserver(in);
  }

void SEntity::removeTreeObserver(STreeObserver *in)
  {
  removeObserver(in);
  }

void SEntity::removeConnectionObserver(SConnectionObserver *in)
  {
  removeObserver(in);
  }

void SEntity::removeObserver(SObserver *in)
  {
  for(int x=0; x<_observers.size(); ++x)
    {
    SObserver *obs = _observers[x].getObserver();
    if(obs == in)
      {
      xRemoveAll(obs->_entities, this);
      _observers.removeAt(x);
      --x;
      }
    }
  }

void SEntity::informDirtyObservers(SProperty *prop)
  {
  SProfileFunction
  foreach(const ObserverStruct &obs, _observers)
    {
    if(obs.mode == ObserverStruct::Dirty)
      {
      SDirtyObserver *observer = ((SDirtyObserver*)obs.observer);
      observer->onPropertyDirtied(prop);
      handler()->currentBlockObserverList() << observer;
      }
    }
  }

void SEntity::informTreeObservers(const SChange *event, bool backwards)
  {
  SProfileFunction
  foreach(const ObserverStruct &obs, _observers)
    {
    if(obs.mode == ObserverStruct::Tree)
      {
      STreeObserver *observer = ((STreeObserver*)obs.observer);
      observer->onTreeChange(event, backwards);
      handler()->currentBlockObserverList() << observer;
      }
    }

  SEntity *parentEnt = parentEntity();
  if(parentEnt)
    {
    parentEnt->informTreeObservers(event, backwards);
    }
  }

void SEntity::informConnectionObservers(const SChange *event, bool backwards)
  {
  SProfileFunction
  foreach(const ObserverStruct &obs, _observers)
    {
    if(obs.mode == ObserverStruct::Connection)
      {
      SConnectionObserver *observer = ((SConnectionObserver*)obs.observer);
      observer->onConnectionChange(event, backwards);
      handler()->currentBlockObserverList() << observer;
      }
    }
  }

