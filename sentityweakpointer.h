#ifndef SENTITYWEAKPOINTER_H
#define SENTITYWEAKPOINTER_H

#include "XWeakShared"
#include "sentity.h"
#include "sdatabase.h"

#define S_ENTITY_WEAK_POINTER_CHECK xAssert(data());

class SEntityWeakPointer : private XWeakSharedPointer<SEntity>
  {
public:
  explicit SEntityWeakPointer(SEntity *ptr=0) : XWeakSharedPointer<SEntity>(ptr)
    {
    }

  SEntity *operator=(SEntity *ptr)
    {
    XWeakSharedPointer<SEntity>::assign(ptr);
    return ptr;
    }

  void assign(SEntity *ptr)
    {
    XWeakSharedPointer<SEntity>::assign(ptr);
    }

  const SEntity *operator->() const
    {
    S_ENTITY_WEAK_POINTER_CHECK
    return data();
    }

  SEntity *operator->()
    {
    S_ENTITY_WEAK_POINTER_CHECK
    return data();
    }

  const SEntity *entity() const
    {
    S_ENTITY_WEAK_POINTER_CHECK
    return data();
    }

  SEntity *entity()
    {
    S_ENTITY_WEAK_POINTER_CHECK
    return data();
    }

  bool isValid() const
    {
    return data() != 0;
    }

  operator SEntity*()
    {
    return data();
    }

  operator const SEntity*() const
    {
    return data();
    }
  };

#define S_ENTITY_TYPED_WEAK_POINTER_CHECK xAssert(SEntityWeakPointer::entity() && SEntityWeakPointer::entity()->castTo<T>(), (xsize)SEntityWeakPointer::entity());

template <typename T> class SEntityTypedWeakPointer : public SEntityWeakPointer
  {
public:
  explicit SEntityTypedWeakPointer(T *ptr=0) : SEntityWeakPointer(ptr)
    {
    }

  SEntity *operator=(T *ptr)
    {
    SEntityWeakPointer::assign(ptr);
    return ptr;
    }

  void assign(T *ptr)
    {
    SEntityWeakPointer::assign(ptr);
    }

  const T *operator->() const
    {
    S_ENTITY_TYPED_WEAK_POINTER_CHECK
    return entity();
    }

  T *operator->()
    {
    S_ENTITY_TYPED_WEAK_POINTER_CHECK
    return entity();
    }

  const T *entity() const
    {
    S_ENTITY_TYPED_WEAK_POINTER_CHECK
    return SEntityWeakPointer::entity()->uncheckedCastTo<T>();
    }

  T *entity()
    {
    S_ENTITY_TYPED_WEAK_POINTER_CHECK
    return SEntityWeakPointer::entity()->uncheckedCastTo<T>();
    }

  operator T*()
    {
    return entity();
    }

  operator const T*() const
    {
    return entity();
    }
  };

inline bool operator!=(SProperty *prop, const SEntityWeakPointer &ptr)
  {
  return prop != ptr.entity();
  }

#endif // SENTITYWEAKPOINTER_H
