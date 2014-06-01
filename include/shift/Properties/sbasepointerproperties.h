#ifndef SBASEPOINTERPROPERTIES_H
#define SBASEPOINTERPROPERTIES_H

#include "shift/sdatabase.h"
#include "shift/TypeInformation/smodule.h"
#include "shift/Properties/sproperty.h"
#include "shift/Properties/sarray.h"
#include "shift/Properties/scontaineriterators.h"

namespace Shift
{

class SHIFT_EXPORT Pointer : public Property
  {
  S_PROPERTY(Pointer, Property);

public:
  typedef Property PtrType;

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

  Property *pointed() { preGet(); return input(); }
  Property *operator()() { preGet(); return pointed(); }
  const Property *pointed() const { preGet(); return input(); }
  const Property *operator()() const { preGet(); return pointed(); }

  void setPointed(const Property *prop);
  Pointer &operator=(const Property *prop) { setPointed(prop); return *this; }

  //const PropertyInformation *pointedToType() const;
  //static const PropertyInformation *pointedToType(const PropertyInformation *);

  //static void assignPointerInformation(PropertyInformation *newInfo, const PropertyInformation *pointedToInfo);
  };

template <typename T> class TypedPointer : public Pointer
  {
public:
  typedef T PtrType;

  // pre gets here to make sure we clear any dirty flags that have flowed in through
  // dependencies
  T *pointed() { preGet(); return input() ? input()->template castTo<T>() : 0; }
  T *operator()() { preGet(); return pointed(); }

  const T *pointed() const { preGet(); return input() ? input()->template castTo<T>() : 0; }
  const T *operator()() const { preGet(); return pointed(); }

  void setPointed(const T *prop) { Pointer::setPointed(prop); }
  Pointer &operator=(const T *prop) { setPointed(prop); return *this; }
  };

template <typename PTR> class TypedPointerArray : public TypedPropertyArray<PTR>
  {
public:
  PTR* addPointer(const typename PTR::PtrType *prop)
    {
    Block b(Property::handler());
    PTR *p = TypedPropertyArray<PTR>::add();
    xAssert(p);
    p->setPointed(prop);

    return p;
    }

  void removePointer(const typename PTR::PtrType *ptr)
    {
    xForeach(auto c, Container::walker<Pointer>())
      {
      if(c)
        {
        if(c->pointed() == ptr)
          {
          TypedPropertyArray<PTR>::remove(c);
          return;
          }
        }
      }
    }

  bool hasPointer(const typename PTR::PtrType *ptr)
    {
    xForeach(auto child, Container::walker<Pointer>())
      {
      if(ptr == child->input())
        {
        return true;
        }
      }
    return false;
    }

  void clear()
    {
    Container::clear();
    }
  };


class PointerComputeLock
  {
public:
  PointerComputeLock(Pointer *p) : _p(p) { }
  ~PointerComputeLock() { _p->pointed(); }

private:
  Pointer *_p;
  };

#define S_TYPED_POINTER_TYPE(exportType, name, type) \
  class exportType name : public Shift::TypedPointer<type> { \
    S_PROPERTY(name, Pointer); }; \
  S_PROPERTY_INTERFACE(name)

#define S_IMPLEMENT_TYPED_POINTER_TYPE(name, group) \
  S_IMPLEMENT_PROPERTY(name, group) \
  void name::createTypeInformation(Shift::PropertyInformationTyped<name> *, \
    const Shift::PropertyInformationCreateData &) { }

template <typename T, typename TYPE> void createTypedPointerArray(
    PropertyInformationTyped<T> *,
    const PropertyInformationCreateData &)
  {
  }

#define S_TYPED_POINTER_ARRAY_TYPE(exportType, name, type) \
  class exportType name : public Shift::TypedPointerArray<type> { \
  S_PROPERTY_CONTAINER(name, Container); }; \

#define S_IMPLEMENT_TYPED_POINTER_ARRAY_TYPE(name, group) \
  S_IMPLEMENT_PROPERTY(name, group) \
  void name::createTypeInformation(Shift::PropertyInformationTyped<name> *info, \
    const Shift::PropertyInformationCreateData &data) { Shift::createTypedPointerArray<name, name::ElementType>(info, data); } \

S_TYPED_POINTER_ARRAY_TYPE(SHIFT_EXPORT, PointerArray, Pointer)

}

S_PROPERTY_INTERFACE(Shift::PointerArray)
S_PROPERTY_INTERFACE(Shift::TypedPointerArray<Shift::Pointer>)
S_PROPERTY_INTERFACE(Shift::Pointer)

#endif // SBASEPOINTERPROPERTIES_H
