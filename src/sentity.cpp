#include "shift/sentity.h"
#include "shift/sdatabase.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"

namespace Shift
{

S_IMPLEMENT_PROPERTY(Entity, Shift)

void Entity::createTypeInformation(PropertyInformationTyped<Entity> *info,
                                    const PropertyInformationCreateData &data)
  {
  auto childBlock = info->createChildrenBlock(data);

  auto *childInst = childBlock.add(&Entity::children, "children");
  childInst->setMode(PropertyInstanceInformation::Internal);
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

Entity::Entity()
#ifdef S_CENTRAL_CHANGE_HANDLER
    : _handler(0)
#else
    : _database(0)
#endif
  {
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

  parent()->moveAttribute(&ent->children, this);
  }

Attribute *Entity::addChild(const PropertyInformation *info, const NameArg& name)
  {
  Attribute *ent = children.add(info, std::numeric_limits<xsize>::max(), name);
  xAssert(ent);
  return ent;
  }

Attribute *Entity::addAttribute(const PropertyInformation *info, xsize index, const NameArg& name, PropertyInstanceInformationInitialiser *inst)
  {
  Attribute *p = Container::addAttribute(info, index, name, inst);
  xAssert(p);
  return p;
  }

const Entity *Entity::parentEntity() const
  {
  const Container *par = parent();
  if(par)
    {
    return par->entity();
    }
  return nullptr;
  }

Entity *Entity::parentEntity()
  {
  Container *par = parent();
  if(par)
    {
    return par->entity();
    }
  return 0;
  }

Entity *Entity::findChildEntity(const NameArg &n)
  {
  Attribute *prop = children.findChild(n);
  if(!prop)
    {
    return nullptr;
    }
  return prop->castTo<Entity>();
  }

const Entity *Entity::findChildEntity(const NameArg &n) const
  {
  const Attribute *prop = children.findChild(n);
  if(!prop)
    {
    return nullptr;
    }
  return prop->castTo<Entity>();
  }

Observer *Entity::ObserverStruct::getObserver()
  {
  if(mode == ObserverStruct::Tree)
    {
    return observer.tree;
    }
  else if(mode == ObserverStruct::Connection)
    {
    return observer.connection;
    }
  return nullptr;
  }

void Entity::setupObservers()
  {
  if(!_observers.allocator().allocator())
    {
    _observers.allocator() = generalPurposeAllocator();
    }
  }

void Entity::addTreeObserver(TreeObserver *in)
  {
  setupObservers();

  xAssert(in);
  ObserverStruct s;
  s.mode = ObserverStruct::Tree;
  s.observer.tree = in;
  in->_entities << this;
  _observers << s;
  }

void Entity::addConnectionObserver(ConnectionObserver *in)
  {
  setupObservers();

  ObserverStruct s;
  s.mode = ObserverStruct::Connection;
  s.observer.connection = in;
  in->_entities << this;
  _observers << s;
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

void Entity::informTreeObservers(const Change *event, bool backwards)
  {
  xForeach(const ObserverStruct &obs, _observers)
    {
    if(obs.mode == ObserverStruct::Tree)
      {
      TreeObserver *observer = obs.observer.tree;
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
  xForeach(const ObserverStruct &obs, _observers)
    {
    if(obs.mode == ObserverStruct::Connection)
      {
      ConnectionObserver *observer = obs.observer.connection;
      observer->onConnectionChange(event, backwards);
      handler()->currentBlockObserverList() << observer;
      }
    }
  }

}
