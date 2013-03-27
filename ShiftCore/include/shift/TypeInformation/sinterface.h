#ifndef SINTERFACE_H
#define SINTERFACE_H

#include "shift/Properties/sproperty.h"
#include "shift/TypeInformation/styperegistry.h"

#define S_INTERFACE_FACTORY_TYPE(interfaceType) public: \
  typedef interfaceType InterfaceType; \
  private:

namespace Shift
{

class InterfaceBase;

class InterfaceBaseFactory
  {
  S_INTERFACE_FACTORY_TYPE(InterfaceBase);

XProperties:
  XProperty(xptrdiff, offset, setOffset);

public:
  virtual ~InterfaceBaseFactory() { }
  virtual InterfaceBase *classInterface(Property *) { return 0; }
  };

#define S_INTERFACE_TYPE(typeId) public: \
  enum { InterfaceTypeId = SInterfaceTypes::typeId }; \
  virtual xuint32 interfaceTypeId() const { return InterfaceTypeId; } \
  private:

class InterfaceBase
#ifdef S_PROPERTY_USER_DATA
    : public Property::UserData
#endif
  {
  S_INTERFACE_TYPE(Invalid)

#ifdef S_PROPERTY_USER_DATA
  bool onPropertyDelete(Property *) X_OVERRIDE { return true; }
#endif
  };


#define S_STATIC_INTERFACE_TYPE(type, interfaceTypeId) public: \
  S_INTERFACE_TYPE(interfaceTypeId) \
  S_INTERFACE_FACTORY_TYPE(type)

class StaticInterfaceBase : public InterfaceBase, public InterfaceBaseFactory
  {
public:
  InterfaceBase *classInterface(Property *) X_OVERRIDE { return this; }
  };


namespace Interface
{

template <typename PropType, typename T> static T *addStaticInterface()
  {
  T* t = TypeRegistry::interfaceAllocator()->create<T>();

  InterfaceBaseFactory *fac = t;
  xptrdiff offset = (xuint8*)t - (xuint8*)fac;

  TypeRegistry::addInterfaceFactory(
        PropType::staticTypeInformation(),
        T::InterfaceType::InterfaceTypeId,
        t,
        offset);
  return t;
  }

template <typename T> static void addStaticInterface(
    PropertyInformation *info,
    T *factory)
  {
  TypeRegistry::addInterfaceFactory(
        info,
        T::InterfaceType::InterfaceTypeId,
        factory);
  }

template <typename T> static T *addStaticInterface(PropertyInformation *info)
  {
  T* t = TypeRegistry::interfaceAllocator()->create<T>();

  InterfaceBaseFactory *fac = t;
  xptrdiff offset = (xuint8*)t - (xuint8*)fac;

  typedef typename T::InterfaceType IfcType;
  TypeRegistry::addInterfaceFactory(
        info,
        IfcType::InterfaceTypeId,
        t,
        offset);
  return t;
  }

template <typename PropType, typename T> static void addInheritedInterface()
  {
  class InheritedInterface : public InterfaceBaseFactory
    {
    S_INTERFACE_FACTORY_TYPE(T)
  public:
    InterfaceBase *classInterface(Property *prop) X_OVERRIDE
      {
      PropType *type = prop->castTo<PropType>();
      T *t = type;
      return t;
      }
    };

  addStaticInterface<PropType, InheritedInterface>();
  }
}
}

#endif // SINTERFACE_H
