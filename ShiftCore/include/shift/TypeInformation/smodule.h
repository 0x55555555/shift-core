#ifndef SMODULE_H
#define SMODULE_H

#include "Containers/XVector.h"
#include "Memory/XUniquePointer.h"
#include "shift/Properties/sproperty.h"
#include "shift/TypeInformation/sinterface.h"

namespace Shift
{

class PropertyInformation;

class ModuleBuilder
  {
public:
  virtual void addType(PropertyInformation *) = 0;
  virtual void removeType(PropertyInformation *) = 0;

  virtual void addInterfaceFactory(const PropertyInformation *info, const InterfaceBaseFactory *factory) = 0;
  virtual void removeInterfaceFactory(const PropertyInformation *info, const InterfaceBaseFactory *factory) = 0;
  };

class SHIFT_EXPORT Module
  {
public:
  struct InterfaceData
    {
    const PropertyInformation *type;
    Eks::UniquePointer<InterfaceBaseFactory> factory;
    };

XProperties:
  XROByRefProperty(Eks::Vector<InterfaceData>, interfaces);

  virtual void initialiseInterfaces(Module &) { }

  template <typename PropType, typename T> T *addStaticInterface()
    {
    auto t = TypeRegistry::interfaceAllocator()->createUnique<T>();

    T *val = t.get();
    _interfaces.emplaceBack(PropType::staticTypeInformation(), std::move(t));
    return val;
    }

  template <typename T> T *addStaticInterface(PropertyInformation *info)
    {
    auto t = TypeRegistry::interfaceAllocator()->createUnique<T>();

    T *val = t.get();
    _interfaces.emplaceBack(info, std::move(t));
    return val;
    }

  template <typename PropType, typename T> void addInheritedInterface()
    {
    class InheritedInterface : public InterfaceBaseFactory
      {
      S_INTERFACE_FACTORY_TYPE(T)
      xuint32 interfaceTypeId() const X_OVERRIDE { return T::InterfaceTypeId; }

    public:
      InterfaceBase *classInterface(Attribute *prop) X_OVERRIDE
        {
        PropType *type = prop->castTo<PropType>();
        T *t = type;
        return t;
        }
      };

    addStaticInterface<PropType, InheritedInterface>();
    }

  typedef const PropertyInformation *(*BootstrapFunction)(Eks::AllocatorBase *allocator);
  struct Information
    {
    PropertyInformation *information;
    BootstrapFunction bootstrap;
    Information *next;
    };

  Module();

  void install(ModuleBuilder *, Eks::AllocatorBase *alloc);
  void uninstall(ModuleBuilder *, Eks::AllocatorBase *alloc);

  Information &registerPropertyInformation(Information &info, BootstrapFunction fn);

private:
  Information *_first;
  };


}

#endif // SMODULE_H
