#ifndef SEXTERNALPOINTER_H
#define SEXTERNALPOINTER_H

#include "shift/Properties/sproperty.h"
#include "shift/sentity.h"
#include "shift/TypeInformation/spropertyinstanceinformation.h"
#include "QUuid"

namespace Shift
{

class ExternalPointer;

class SHIFT_EXPORT ExternalPointerInstanceInformation : public EmbeddedPropertyInstanceInformation
  {
public:
  enum ResolveResult
    {
    DataDoesntExist,
    DataNotReady,
    DataIncorrectType,
    Success
    };

  typedef const Property *(*ResolveExternalPointer)(const ExternalPointer *,
                                                     const ExternalPointerInstanceInformation *inst,
                                                     ResolveResult *res);


XProperties:
  XProperty(ResolveExternalPointer, resolveFunction, setResolveFunction);

public:
  ExternalPointerInstanceInformation();

  };

class SHIFT_EXPORT ExternalPointer : public Property
  {
public:
  typedef ExternalPointerInstanceInformation EmbeddedInstanceInformation;

  typedef EmbeddedInstanceInformation::ResolveResult ResolveResult;

private:
  S_PROPERTY(ExternalPointer, Property, 0)

public:

  Property *resolve(ResolveResult *result=0);
  const Property *resolve(ResolveResult *result=0) const;


  template <typename T>
  const T *pointed() const
    {
    const Property *p = pointed();
    if(p)
      {
      return p->castTo<T>();
      }
    return 0;
    }

  template <typename T>
  T *pointed()
    {
    Property *p = pointed();
    if(p)
      {
      return p->castTo<T>();
      }
    return 0;
    }

  Property *pointed() { return resolve(); }
  Property *operator()() { return resolve(); }
  };

class SUuidEntity;
class SHIFT_EXPORT ExternalUuidPointer : public ExternalPointer
  {
  S_PROPERTY(ExternalUuidPointer, ExternalPointer, 0)

public:
  class Traits;

  void setPointed(const SUuidEntity *entity);

  const QUuid &uuid() const { return _id; }

private:
  QUuid _id;
  friend class SUuidEntity;
  };

class SHIFT_EXPORT SUuidEntity : public Entity
  {
  S_ENTITY(SUuidEntity, Entity, 0)

public:
  SUuidEntity();

  const QUuid &uuid() const { return _id; }

private:
  QUuid _id;
  friend class ExternalUuidPointer;
  };

}

S_PROPERTY_INTERFACE(Shift::ExternalPointer)
S_PROPERTY_INTERFACE(Shift::ExternalUuidPointer)
S_PROPERTY_INTERFACE(Shift::SUuidEntity)

#endif // SEXTERNALPOINTER_H
