#ifndef SENTITY_H
#define SENTITY_H

#include "sglobal.h"
#include "spropertycontainer.h"
#include "spropertyarray.h"
#include "sobserver.h"
#include "XWeakShared"

class SProperty;
class SDatabase;
class STreeObserver;
class SDirtyObserver;
class SConnectionObserver;

#define S_ENTITY(name, parent, version) S_PROPERTY_CONTAINER(name, parent, version)
#define S_ABSTRACT_ENTITY(name, parent, version) S_ABSTRACT_PROPERTY_CONTAINER(name, parent, version)

class SHIFT_EXPORT SEntity : public SPropertyContainer, public XWeakSharedData
  {
  S_ENTITY(SEntity, SPropertyContainer, 0);

public:
  ~SEntity();

  SPropertyArray children;

  void reparent(SEntity *);

  template <typename T>T *addChild(const QString& name="")
    {
    SProperty *p = addChild(T::staticTypeInformation(), name);
    xAssert(p);
    return p->uncheckedCastTo<T>();
    }

  SProperty *addChild(const SPropertyInformation *info, const QString& name="");

  template <typename T>T *addProperty(const QString& name="", SPropertyInstanceInformationInitialiser *init=0)
    {
    SProperty *p = addProperty(T::staticTypeInformation(), name, init);
    xAssert(p);

    return p->uncheckedCastTo<T>();
    }

  SProperty *addProperty(const SPropertyInformation *info, const QString& name="", SPropertyInstanceInformationInitialiser *inst=0);

  void removeProperty(SProperty *prop)
    {
    SPropertyContainer::removeProperty(prop);
    }

  void moveProperty(SEntity *newParent, SProperty *prop)
    {
    SPropertyContainer::moveProperty(newParent, prop);
    }

  // accessing child properties by index
  using SPropertyContainer::at;

  SEntity *parentEntity() const;

  void addDirtyObserver(SDirtyObserver *);
  void addTreeObserver(STreeObserver *);
  void addConnectionObserver(SConnectionObserver *);

  void removeDirtyObserver(SDirtyObserver *);
  void removeTreeObserver(STreeObserver *);
  void removeConnectionObserver(SConnectionObserver *);
  void removeObserver(SObserver *);

  void informDirtyObservers(SProperty *prop);
  void informTreeObservers(const SChange *event, bool backwards);
  void informConnectionObservers(const SChange *event, bool backwards);

  SEntity *findChildEntity(const QString &);
  const SEntity *findChildEntity(const QString &) const;

  static void saveProperty(const SProperty *, SSaver &);
  static SProperty *loadProperty(SPropertyContainer *, SLoader &);

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

    SObserver *getObserver();
    };
  XList<ObserverStruct> _observers;
  };

S_PROPERTY_INTERFACE(SEntity)

#endif // SENTITY_H
