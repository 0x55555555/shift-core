#ifndef STYPEREGISTRY_H
#define STYPEREGISTRY_H

#include "shift/sglobal.h"
#include "Containers/XVector.h"

namespace Eks
{
class TemporaryAllocatorCore;
}

namespace Shift
{

class Module;
class NameArg;
class PropertyInformation;
class InterfaceBaseFactory;

class SHIFT_EXPORT TypeRegistry
  {
public:
  TypeRegistry(Eks::AllocatorBase *allocator);
  ~TypeRegistry();

  void installModule(Module &);
  void uninstallModule(Module &);

  class Observer
    {
  public:
    virtual void typeAdded(const PropertyInformation *) = 0;
    virtual void typeRemoved(const PropertyInformation *) = 0;
    };

  static void addTypeObserver(Observer *o);
  static void removeTypeObserver(Observer *o);

  static Eks::AllocatorBase *persistentBlockAllocator();
  static Eks::AllocatorBase *generalPurposeAllocator();
  static Eks::AllocatorBase *interfaceAllocator();
  static Eks::TemporaryAllocatorCore *temporaryAllocator();

  static const Eks::Vector<const PropertyInformation *> &types();

  static const PropertyInformation *findType(const NameArg &);

  static const Eks::String& getModeString(xsize mode);
  static xsize getModeFromString(const Eks::String& mode);

  static const InterfaceBaseFactory *interfaceFactory(
      const PropertyInformation *info,
      xuint32 typeId);

private:
  X_DISABLE_COPY(TypeRegistry);

  static void internalAddType(PropertyInformation *);

  friend class PropertyGroup;
  };

}

#endif // STYPEREGISTRY_H
