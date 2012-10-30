#ifndef SEXTERNALPOINTER_H
#define SEXTERNALPOINTER_H

#include "sproperty.h"
#include "sentity.h"
#include "spropertyinstanceinformation.h"
#include "QUuid"

class ExternalPointer;

class SHIFT_EXPORT ExternalPointerInstanceInformation : public SEmbeddedPropertyInstanceInformation
  {
public:
  enum ResolveResult
    {
    DataDoesntExist,
    DataNotReady,
    DataIncorrectType,
    Success
    };

  typedef SProperty *(*ResolveExternalPointer)(const ExternalPointer *,
                                               const ExternalPointerInstanceInformation *inst,
                                               ResolveResult *res);


XProperties:
  XProperty(ResolveExternalPointer, resolveFunction, setResolveFunction);

public:
  ExternalPointerInstanceInformation();

  };

class SHIFT_EXPORT ExternalPointer : public SProperty
  {
public:
  typedef ExternalPointerInstanceInformation EmbeddedInstanceInformation;

  typedef EmbeddedInstanceInformation::ResolveResult ResolveResult;

private:
  S_PROPERTY(ExternalPointer, SProperty, 0)

public:

  SProperty *resolve(ResolveResult *result=0);
  const SProperty *resolve(ResolveResult *result=0) const;


  template <typename T>
  const T *pointed() const
    {
    const SProperty *p = pointed();
    if(p)
      {
      return p->castTo<T>();
      }
    return 0;
    }

  template <typename T>
  T *pointed()
    {
    SProperty *p = pointed();
    if(p)
      {
      return p->castTo<T>();
      }
    return 0;
    }

  SProperty *pointed() { return resolve(); }
  SProperty *operator()() { return resolve(); }
  };

class SUuidEntity;
class SHIFT_EXPORT ExternalUuidPointer : public ExternalPointer
  {
  S_PROPERTY(ExternalUuidPointer, ExternalPointer, 0)

public:
  void setPointed(const SUuidEntity *entity);

private:
  QUuid _id;
  friend class SUuidEntity;
  };

class SHIFT_EXPORT SUuidEntity : public SEntity
  {
  S_ENTITY(SUuidEntity, SEntity, 0)

public:

private:
  QUuid _id;
  friend class ExternalUuidPointer;
  };

S_PROPERTY_INTERFACE(ExternalPointer)
S_PROPERTY_INTERFACE(ExternalUuidPointer)
S_PROPERTY_INTERFACE(SUuidEntity)

#endif // SEXTERNALPOINTER_H
