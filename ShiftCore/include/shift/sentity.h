#ifndef SENTITY_H
#define SENTITY_H

#include "shift/sglobal.h"
#include "shift/Properties/spropertycontainer.h"
#include "shift/Properties/spropertyarray.h"
#include "shift/Changes/sobserver.h"
#include "XWeakShared"

namespace Shift
{

#define S_ENTITY(name, parent, version) S_PROPERTY_CONTAINER(name, parent, version)
#define S_ABSTRACT_ENTITY(name, parent, version) S_ABSTRACT_PROPERTY_CONTAINER(name, parent, version)

class SHIFT_EXPORT Entity : public PropertyContainer, public XWeakSharedData
  {
  S_ENTITY(Entity, PropertyContainer, 0);

public:
  ~Entity();

  PropertyArray children;

  void reparent(Entity *);

  template <typename T>T *addChild(const PropertyNameArg& name="")
    {
    Property *p = addChild(T::staticTypeInformation(), name);
    xAssert(p);
    return p->uncheckedCastTo<T>();
    }

  Property *addChild(const PropertyInformation *info, const PropertyNameArg& name=PropertyNameArg());

  template <typename T>T *addProperty(const PropertyNameArg& name="", PropertyInstanceInformationInitialiser *init=0)
    {
    Property *p = addProperty(T::staticTypeInformation(), name, init);
    xAssert(p);

    return p->uncheckedCastTo<T>();
    }

  Property *addProperty(const PropertyInformation *info, const PropertyNameArg& name=PropertyNameArg(), PropertyInstanceInformationInitialiser *inst=0);

  void removeProperty(Property *prop)
    {
    PropertyContainer::removeProperty(prop);
    }

  void moveProperty(Entity *newParent, Property *prop)
    {
    PropertyContainer::moveProperty(newParent, prop);
    }

  // accessing child properties by index
  using PropertyContainer::at;

  const Entity *parentEntity() const;
  Entity *parentEntity();

  void addDirtyObserver(DirtyObserver *);
  void addTreeObserver(TreeObserver *);
  void addConnectionObserver(ConnectionObserver *);

  void removeDirtyObserver(DirtyObserver *);
  void removeTreeObserver(TreeObserver *);
  void removeConnectionObserver(ConnectionObserver *);
  void removeObserver(Observer *);

  void informDirtyObservers(Property *prop);
  void informTreeObservers(const Change *event, bool backwards);
  void informConnectionObservers(const Change *event, bool backwards);

  Entity *findChildEntity(const PropertyNameArg &);
  const Entity *findChildEntity(const PropertyNameArg &) const;

  X_ALIGNED_OPERATOR_NEW

private:
  struct ObserverStruct
    {
    enum
      {
      Dirty,
      Tree,
      Connection
      };
    xuint8 mode;
    void *observer;

    Observer *getObserver();
    };
  Eks::Vector<ObserverStruct> _observers;
  };

}

S_PROPERTY_INTERFACE(Shift::Entity)

#endif // SENTITY_H
