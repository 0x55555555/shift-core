#ifndef STYPEREGISTRY_H
#define STYPEREGISTRY_H

#include "shift/sglobal.h"

namespace Eks
{
class TemporaryAllocatorCore;
}

namespace Shift
{

class NameArg;
class PropertyGroup;
class PropertyInformation;
class InterfaceBaseFactory;

class SHIFT_EXPORT TypeRegistry
  {
public:
  class Observer
    {
  public:
    virtual void typeAdded(const PropertyInformation *) = 0;
    virtual void typeRemoved(const PropertyInformation *) = 0;
    };

  static void addTypeObserver(Observer *o);
  static void removeTypeObserver(Observer *o);

  static void initiate(Eks::AllocatorBase *allocator);
  static void terminate();

  static Eks::AllocatorBase *persistentBlockAllocator();
  static Eks::AllocatorBase *generalPurposeAllocator();
  static Eks::AllocatorBase *interfaceAllocator();
  static Eks::TemporaryAllocatorCore *temporaryAllocator();

  static const Eks::Vector<const PropertyGroup *> &groups();
  static const Eks::Vector<const PropertyInformation *> &types();

  static void addPropertyGroup(PropertyGroup &);
  static void addType(PropertyInformation *);

  static const PropertyInformation *findType(const NameArg &);


  static const InterfaceBaseFactory *interfaceFactory(
      const PropertyInformation *info,
      xuint32 typeId);

  static void addInterfaceFactory(const PropertyInformation *info,
      xuint32 typeId,
      InterfaceBaseFactory *factory, xptrdiff offset);

private:
  TypeRegistry();
  X_DISABLE_COPY(TypeRegistry);

  static void internalAddType(PropertyInformation *);

  friend class PropertyGroup;
  };

/*
  const InterfaceBaseFactory *interfaceFactory(xuint32 type) const;

  template <typename T> void addInterfaceFactory(T *factory)
    {
    addInterfaceFactoryInternal(T::InterfaceType::InterfaceTypeId, factory);
    }
  template <typename T> void addStaticInterface(T *factory)
    {
    addInterfaceFactory(factory);
    }
  void addInterfaceFactoryInternal(xuint32 typeId, InterfaceBaseFactory *);

#ifdef S_PROPERTY_USER_DATA
  template <typename T> void addAddonInterface() const;
#endif



  template <typename T> void addInheritedInterface()
    {
    class InheritedInterface : public InterfaceBaseFactory
      {
      S_INTERFACE_FACTORY_TYPE(T)
    public:
      InheritedInterface() : InterfaceBaseFactory(true) { }
      virtual InterfaceBase *classInterface(Property *prop)
        {
        return prop->castTo<PropType>();
        }
      };

    PropertyInformation::addInterfaceFactoryInternal(InheritedInterface::InterfaceType::InterfaceTypeId, new InheritedInterface);
    }

void PropertyInformation::addInterfaceFactoryInternal(xuint32 typeId, InterfaceBaseFactory *factory)
  {
  xAssert(factory);
  xAssert(typeId != SInterfaceTypes::Invalid);

  _interfaceFactories.insert(typeId, factory);

  factory->setReferenceCount(factory->referenceCount() + 1);
  xAssert(interfaceFactory(typeId) == factory);
  }

const InterfaceBaseFactory *PropertyInformation::interfaceFactory(xuint32 type) const
  {
  const InterfaceBaseFactory *fac = 0;
  const PropertyInformation *info = this;
  while(!fac && info)
    {
    fac = info->_interfaceFactories.value(type, 0);
    info = info->parentTypeInformation();
    }

  return fac;
  }
*/

}

#endif // STYPEREGISTRY_H
