#ifndef SBASEPOINTERPROPERTIES_H
#define SBASEPOINTERPROPERTIES_H

#include "sproperty.h"
#include "sdatabase.h"
#include "sinterfaces.h"
#include "spropertycontaineriterators.h"

class SHIFT_EXPORT Pointer : public SProperty
  {
  S_PROPERTY(Pointer, SProperty, 0);

public:
  typedef SProperty PtrType;

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

  SProperty *pointed() { preGet(); return input(); }
  const SProperty *pointed() const { preGet(); return input(); }
  const SProperty *operator()() const { preGet(); return pointed(); }

  void setPointed(const SProperty *prop);
  Pointer &operator=(const SProperty *prop) { setPointed(prop); return *this; }

  const SPropertyInformation *pointedToType() const;
  static const SPropertyInformation *pointedToType(const SPropertyInformation *);

  static void assignPointerInformation(SPropertyInformation *newInfo, const SPropertyInformation *pointedToInfo);
  };

template <typename T> class TypedPointer : public Pointer
  {
public:
  typedef T PtrType;

  // pre gets here to make sure we clear any dirty flags that have flowed in through
  // dependencies
  T *pointed() { preGet(); return input() ? input()->castTo<T>() : 0; }
  T *operator()() { preGet(); return pointed(); }

  const T *pointed() const { preGet(); return input() ? input()->castTo<T>() : 0; }
  const T *operator()() const { preGet(); return pointed(); }

  void setPointed(const T *prop) { Pointer::setPointed(prop); }
  Pointer &operator=(const T *prop) { setPointed(prop); return *this; }
  };

template <typename PTR> class TypedPointerArray : public STypedPropertyArray<PTR>
  {
public:
  PTR* addPointer(const typename PTR::PtrType *prop)
    {
    SBlock b(SProperty::handler());
    PTR *p = STypedPropertyArray<PTR>::add();
    xAssert(p);
    p->setPointed(prop);

    return p;
    }

  void removePointer(const typename PTR::PtrType *ptr)
    {
    xForeach(auto c, SPropertyContainer::walker<Pointer>())
      {
      if(c)
        {
        if(c->pointed() == ptr)
          {
          STypedPropertyArray<PTR>::remove(c);
          return;
          }
        }
      }
    }

  bool hasPointer(const typename PTR::PtrType *ptr)
    {
    xForeach(auto child, SPropertyContainer::walker())
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
    SPropertyContainer::clear();
    }
  };


class SPointerComputeLock
  {
public:
  SPointerComputeLock(Pointer *p) : _p(p) { }
  ~SPointerComputeLock() { _p->pointed(); }

private:
  Pointer *_p;
  };

S_PROPERTY_INTERFACE(Pointer)

#define S_TYPED_POINTER_TYPE(exportType, name, type) \
  class exportType name : public TypedPointer<type> { \
    S_PROPERTY(name, Pointer, 0); }; \
  S_PROPERTY_INTERFACE(name)

#define S_IMPLEMENT_TYPED_POINTER_TYPE(name, group) \
  S_IMPLEMENT_PROPERTY(name, group) \
  void name::createTypeInformation(SPropertyInformationTyped<name> *info, \
    const SPropertyInformationCreateData &data) { \
  if(data.registerInterfaces) { \
  assignPointerInformation(info, name::PtrType::staticTypeInformation()); } } \

template <typename T, typename TYPE> void createTypedPointerArray(SPropertyInformation *info,
                                                                  const SPropertyInformationCreateData &data)
  {
  if(data.registerInterfaces)
    {
    typedef typename TYPE::PtrType PtrType;

    class PointerArrayConnectionInterface : public SPropertyConnectionInterface
      {
    public:
      PointerArrayConnectionInterface() : SPropertyConnectionInterface(true) { }
      virtual void connect(SProperty *driven, const SProperty *driver) const
        {
        SBlock b(driven->database());

        T* arr = driven->castTo<T>();
        xAssert(arr);

        const PtrType* ptr = driver->castTo<PtrType>();
        if(ptr)
          {
          arr->addPointer(ptr);
          }
        else
          {
          arr->setInput(driver);
          }
        }
      };

    info->addInterfaceFactoryInternal(PointerArrayConnectionInterface::InterfaceType::InterfaceTypeId, new PointerArrayConnectionInterface);

    auto api = info->apiInterface();



    typedef XScript::MethodToInCa<TypedPointerArray<TYPE>, void (), &T::clear> Fn1;
    api->addFunction("clear", 1, Fn1::Arity, Fn1::Call, Fn1::CallDart);

    typedef XScript::MethodToInCa<TypedPointerArray<TYPE>, TYPE *(const PtrType *), &T::addPointer> Fn2;
    api->addFunction("addPointer", 1, Fn2::Arity, Fn2::Call, Fn2::CallDart);

    typedef XScript::MethodToInCa<TypedPointerArray<TYPE>, bool (const PtrType *), &T::hasPointer> Fn3;
    api->addFunction("hasPointer", 1, Fn3::Arity, Fn3::Call, Fn3::CallDart);

    typedef XScript::MethodToInCa<TypedPointerArray<TYPE>, void (const PtrType *), &T::removePointer> Fn4;
    api->addFunction("removePointer", 1, Fn4::Arity, Fn4::Call, Fn4::CallDart);
    }
  }

#define S_TYPED_POINTER_ARRAY_TYPE(exportType, name, type) \
  class exportType name : public TypedPointerArray<type> { \
  S_PROPERTY_CONTAINER(name, SPropertyContainer, 0); }; \
  S_PROPERTY_INTERFACE(name) \
  S_PROPERTY_INTERFACE(TypedPointerArray<type>)

#define S_IMPLEMENT_TYPED_POINTER_ARRAY_TYPE(name, group) \
  S_IMPLEMENT_PROPERTY(name, group) \
  void name::createTypeInformation(SPropertyInformationTyped<name> *info, const SPropertyInformationCreateData &data) { createTypedPointerArray<name, name::ElementType>(info, data); } \

S_TYPED_POINTER_ARRAY_TYPE(SHIFT_EXPORT, PointerArray, Pointer)

#endif // SBASEPOINTERPROPERTIES_H
