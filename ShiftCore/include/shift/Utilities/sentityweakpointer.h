#ifndef SENTITYWEAKPOINTER_H
#define SENTITYWEAKPOINTER_H

#include "Memory/XWeakShared.h"
#include "shift/sentity.h"
#include "shift/sdatabase.h"

#define S_ENTITY_WEAK_POINTER_CHECK xAssert(Eks::WeakSharedPointer<T>::data());

namespace Shift
{

template <typename T> class EntityWeakPointer : private Eks::WeakSharedPointer<T>
  {
public:
  explicit EntityWeakPointer(T *ptr=0) : Eks::WeakSharedPointer<T>(ptr)
    {
    }

  T *operator=(T *ptr)
    {
    Eks::WeakSharedPointer<T>::assign(ptr);
    return ptr;
    }

  void assign(T *ptr)
    {
    Eks::WeakSharedPointer<T>::assign(ptr);
    }

  const T *operator->() const
    {
    S_ENTITY_WEAK_POINTER_CHECK
    return Eks::WeakSharedPointer<T>::data();
    }

  T *operator->()
    {
    S_ENTITY_WEAK_POINTER_CHECK
    return Eks::WeakSharedPointer<T>::data();
    }

  const T *entity() const
    {
    S_ENTITY_WEAK_POINTER_CHECK
    return Eks::WeakSharedPointer<T>::data();
    }

  T *entity()
    {
    S_ENTITY_WEAK_POINTER_CHECK
    return Eks::WeakSharedPointer<T>::data();
    }

  bool isValid() const
    {
    return Eks::WeakSharedPointer<T>::data() != 0;
    }

  operator T*()
    {
    return Eks::WeakSharedPointer<T>::data();
    }

  operator const T*() const
    {
    return Eks::WeakSharedPointer<T>::data();
    }
  };

}

#endif // SENTITYWEAKPOINTER_H
