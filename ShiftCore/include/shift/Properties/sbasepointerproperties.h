#ifndef SBASEPOINTERPROPERTIES_H
#define SBASEPOINTERPROPERTIES_H

#include "shift/sdatabase.h"
#include "shift/Properties/sproperty.h"
#include "shift/TypeInformation/sinterfaces.h"
#include "shift/Properties/spropertycontaineriterators.h"

namespace Shift
{

class SHIFT_EXPORT Pointer : public Property
  {
  S_PROPERTY(Pointer, Property, 0);

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

  static void assignPointerInformation(PropertyInformation *newInfo, const PropertyInformation *pointedToInfo);
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
    Block b(Property::handler());
    PTR *p = STypedPropertyArray<PTR>::add();
    xAssert(p);
    p->setPointed(prop);

    return p;
    }

  void removePointer(const typename PTR::PtrType *ptr)
    {
    xForeach(auto c, PropertyContainer::walker<Pointer>())
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
    xForeach(auto child, PropertyContainer::walker())
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
    PropertyContainer::clear();
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
    S_PROPERTY(name, Pointer, 0); }; \
  S_PROPERTY_INTERFACE(name)

#define S_IMPLEMENT_TYPED_POINTER_TYPE(name, group) \
  S_IMPLEMENT_PROPERTY(name, group) \
  void name::createTypeInformation(Shift::PropertyInformationTyped<name> *info, \
    const Shift::PropertyInformationCreateData &data) { \
  if(data.registerInterfaces) { \
  assignPointerInformation(info, name::PtrType::staticTypeInformation()); } } \

template <typename T, typename TYPE> void createTypedPointerArray(PropertyInformationTyped<T> *info,
                                                                  const PropertyInformationCreateData &data)
  {
  if(data.registerInterfaces)
    {
    typedef typename TYPE::PtrType PtrType;

    class PointerArrayConnectionInterface : public PropertyConnectionInterface
      {
    public:
      virtual void connect(Property *driven, const Property *driver) const
        {
        Block b(driven->database());

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

    Interface::addStaticInterface<PointerArrayConnectionInterface>(info);

    typedef XScript::MethodToInCa
        <TypedPointerArray<TYPE>, void (), &T::clear>
        ClearType;

    typedef XScript::MethodToInCa
        <TypedPointerArray<TYPE>, TYPE *(const PtrType *), &T::addPointer>
        AddType;

    typedef XScript::MethodToInCa
        <TypedPointerArray<TYPE>, bool (const PtrType *), &T::hasPointer>
        HasType;

    typedef XScript::MethodToInCa
        <TypedPointerArray<TYPE>, void (const PtrType *), &T::removePointer>
        RemoveType;

    XScript::InterfaceBase* api = info->apiInterface();;

    XScript::ClassDef<0,0,4> cls = {
      {
        api->method<ClearType>("clear"),
        api->method<AddType>("addPointer"),
        api->method<HasType>("hasPointer"),
        api->method<RemoveType>("removePointer"),
      }
    };

    api->buildInterface(cls);
    }
  }

#define S_TYPED_POINTER_ARRAY_TYPE(exportType, name, type) \
  class exportType name : public Shift::TypedPointerArray<type> { \
  S_PROPERTY_CONTAINER(name, PropertyContainer, 0); }; \

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
