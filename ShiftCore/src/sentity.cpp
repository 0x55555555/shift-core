#include "shift/sentity.h"
#include "shift/sdatabase.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/sinterfaces.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "QStringList"

namespace Shift
{

S_IMPLEMENT_PROPERTY(Entity, Shift)

void Entity::createTypeInformation(PropertyInformationTyped<Entity> *info,
                                    const PropertyInformationCreateData &data)
  {
  if(data.registerAttributes)
    {
    auto *childInst = info->add(data, &Entity::children, "children");
    childInst->setMode(PropertyInstanceInformation::Internal);
    }

  if(data.registerInterfaces)
    {
    XScript::Interface<Entity> *api = info->apiInterface();

    static XScript::ClassDef<0,0,7> cls = {
      {
      api->method<Property* (const PropertyInformation *, const PropertyNameArg &), &Entity::addChild>("addChild"),

      api->method<void (TreeObserver*), &Entity::addTreeObserver>("addTreeObserver"),
      api->method<void (DirtyObserver*), &Entity::addDirtyObserver>("addDirtyObserver"),
      api->method<void (ConnectionObserver*), &Entity::addConnectionObserver>("addConnectionObserver"),

      api->method<void (TreeObserver*), &Entity::removeTreeObserver>("removeTreeObserver"),
      api->method<void (DirtyObserver*), &Entity::removeDirtyObserver>("removeDirtyObserver"),
      api->method<void (ConnectionObserver*), &Entity::removeConnectionObserver>("removeConnectionObserver")
      }
    };

    api->buildInterface(cls);
    }
  }

template <typename T> void xRemoveAll(Eks::Vector<T>& vec, const T &ptr)
  {
  for(xsize i = 0; i < vec.size(); ++i)
    {
    if(vec[i] == ptr)
      {
      vec.removeAt(i--);
      }
    }
  }

Entity::~Entity()
  {
  for(xsize o = 0; o < _observers.size(); ++o)
    {
    Observer* observer = _observers[o].getObserver();
    Eks::Vector<Entity*>& ents = observer->_entities;
    xRemoveAll(ents, this);
    }
  _observers.clear();
  }

void Entity::reparent(Entity *ent)
  {
  xAssert(parent() && parentEntity());

  parent()->moveProperty(&ent->children, this);
  }

Property *Entity::addChild(const PropertyInformation *info, const PropertyNameArg& name)
  {
  Block b(handler());
  Property *ent = children.add(info, X_SIZE_SENTINEL, name);
  xAssert(ent);
  return ent;
  }

Property *Entity::addProperty(const PropertyInformation *info, const PropertyNameArg& name, PropertyInstanceInformationInitialiser *inst)
  {
  Block b(handler());
  Property *p = PropertyContainer::addProperty(info, X_SIZE_SENTINEL, name, inst);
  xAssert(p);
  return p;
  }

const Entity *Entity::parentEntity() const
  {
  const PropertyContainer *par = parent();
  if(par)
    {
    return par->entity();
    }
  return 0;
  }

Entity *Entity::parentEntity()
  {
  PropertyContainer *par = parent();
  if(par)
    {
    return par->entity();
    }
  return 0;
  }

Entity *Entity::findChildEntity(const PropertyNameArg &n)
  {
  Property *prop = children.findChild(n);
  if(!prop)
    {
    return 0;
    }
  return prop->castTo<Entity>();
  }

const Entity *Entity::findChildEntity(const PropertyNameArg &n) const
  {
  const Property *prop = children.findChild(n);
  if(!prop)
    {
    return 0;
    }
  return prop->castTo<Entity>();
  }

Observer *Entity::ObserverStruct::getObserver()
  {
  if(mode == ObserverStruct::Tree)
    {
    return (TreeObserver*)observer;
    }
  else if(mode == ObserverStruct::Dirty)
    {
    return (DirtyObserver*)observer;
    }
  else if(mode == ObserverStruct::Connection)
    {
    return (ConnectionObserver*)observer;
    }
  return 0;
  }

void Entity::addDirtyObserver(DirtyObserver *in)
  {
  ObserverStruct s;
  s.mode = ObserverStruct::Dirty;
  s.observer = in;
  in->_entities << this;
  _observers << s;
  }

void Entity::addTreeObserver(TreeObserver *in)
  {
  xAssert(in);
  ObserverStruct s;
  s.mode = ObserverStruct::Tree;
  s.observer = in;
  in->_entities << this;
  _observers << s;
  }

void Entity::addConnectionObserver(ConnectionObserver *in)
  {
  ObserverStruct s;
  s.mode = ObserverStruct::Connection;
  s.observer = in;
  in->_entities << this;
  _observers << s;
  }

void Entity::removeDirtyObserver(DirtyObserver *in)
  {
  removeObserver(in);
  }

void Entity::removeTreeObserver(TreeObserver *in)
  {
  removeObserver(in);
  }

void Entity::removeConnectionObserver(ConnectionObserver *in)
  {
  removeObserver(in);
  }

void Entity::removeObserver(Observer *in)
  {
  for(xsize x=0; x<_observers.size(); ++x)
    {
    Observer *obs = _observers[x].getObserver();
    if(obs == in)
      {
      xRemoveAll(obs->_entities, this);
      _observers.removeAt(x);
      --x;
      }
    }
  }

void Entity::informDirtyObservers(Property *prop)
  {
  SProfileFunction
  Q_FOREACH(const ObserverStruct &obs, _observers)
    {
    if(obs.mode == ObserverStruct::Dirty)
      {
      DirtyObserver *observer = ((DirtyObserver*)obs.observer);
      observer->onPropertyDirtied(prop);
      handler()->currentBlockObserverList() << observer;
      }
    }
  }

void Entity::informTreeObservers(const Change *event, bool backwards)
  {
  SProfileFunction
  Q_FOREACH(const ObserverStruct &obs, _observers)
    {
    if(obs.mode == ObserverStruct::Tree)
      {
      TreeObserver *observer = ((TreeObserver*)obs.observer);
      observer->onTreeChange(event, backwards);
      handler()->currentBlockObserverList() << observer;
      }
    }

  Entity *parentEnt = parentEntity();
  if(parentEnt)
    {
    parentEnt->informTreeObservers(event, backwards);
    }
  }

void Entity::informConnectionObservers(const Change *event, bool backwards)
  {
  SProfileFunction
  Q_FOREACH(const ObserverStruct &obs, _observers)
    {
    if(obs.mode == ObserverStruct::Connection)
      {
      ConnectionObserver *observer = ((ConnectionObserver*)obs.observer);
      observer->onConnectionChange(event, backwards);
      handler()->currentBlockObserverList() << observer;
      }
    }
  }

}
