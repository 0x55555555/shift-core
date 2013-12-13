#ifndef SENTITYWEAKPOINTER_H
#define SENTITYWEAKPOINTER_H

#include "Memory/XWeakShared.h"
#include "shift/sentity.h"
#include "shift/sdatabase.h"

#define S_ENTITY_WEAK_POINTER_CHECK xAssert(data());

namespace Shift
{

class EntityWeakPointer : private XWeakSharedPointer<Entity>
  {
public:
  explicit EntityWeakPointer(Entity *ptr=0) : XWeakSharedPointer<Entity>(ptr)
    {
    }

  Entity *operator=(Entity *ptr)
    {
    XWeakSharedPointer<Entity>::assign(ptr);
    return ptr;
    }

  void assign(Entity *ptr)
    {
    XWeakSharedPointer<Entity>::assign(ptr);
    }

  const Entity *operator->() const
    {
    S_ENTITY_WEAK_POINTER_CHECK
    return data();
    }

  Entity *operator->()
    {
    S_ENTITY_WEAK_POINTER_CHECK
    return data();
    }

  const Entity *entity() const
    {
    S_ENTITY_WEAK_POINTER_CHECK
    return data();
    }

  Entity *entity()
    {
    S_ENTITY_WEAK_POINTER_CHECK
    return data();
    }

  bool isValid() const
    {
    return data() != 0;
    }

  operator Entity*()
    {
    return data();
    }

  operator const Entity*() const
    {
    return data();
    }
  };

#define S_ENTITY_TYPED_WEAK_POINTER_CHECK xAssert(EntityWeakPointer::entity() && ((Entity*)EntityWeakPointer::entity())->castTo<T>(), (xsize)EntityWeakPointer::entity());

template <typename T> class EntityTypedWeakPointer : public EntityWeakPointer
  {
public:
  explicit EntityTypedWeakPointer(T *ptr=0) : EntityWeakPointer(ptr)
    {
    }

  Entity *operator=(T *ptr)
    {
    EntityWeakPointer::assign(ptr);
    return ptr;
    }

  void assign(T *ptr)
    {
    EntityWeakPointer::assign(ptr);
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
    Entity *ent = EntityWeakPointer::entity();
    return ent->uncheckedCastTo<T>();
    }

  T *entity()
    {
    S_ENTITY_TYPED_WEAK_POINTER_CHECK
    Entity *ent = EntityWeakPointer::entity();
    return ent->uncheckedCastTo<T>();
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

inline bool operator!=(Property *prop, const EntityWeakPointer &ptr)
  {
  return prop != ptr.entity();
  }

}

#endif // SENTITYWEAKPOINTER_H
