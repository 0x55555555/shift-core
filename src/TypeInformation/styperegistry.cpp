#include "Memory/XBucketAllocator.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertyinstanceinformation.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/smodule.h"
#include "shift/Properties/sbaseproperties.h"
#include "shift/Properties/scontainer.inl"
#include "shift/Properties/sdata.inl"
#include "shift/Properties/sbasepointerproperties.h"
#include "shift/Changes/sobserver.h"
#include "shift/QtExtensions/sinterfaces.h"
#include "Memory/XTemporaryAllocator.h"
#include "Containers/XUnorderedMap.h"
#include "Utilities/XTemplateHelpers.h"

namespace Shift
{

struct TypeData
  {
  enum
    {
    DefaultAllocation = 128,
    ExpandingAllocation = 1024
    };
  TypeData(Eks::AllocatorBase *allocator)
    : modules(allocator),
      types(allocator),
      observers(allocator),
      bucketAllocator(allocator, DefaultAllocation, ExpandingAllocation),
      interfaces(allocator),
      baseAllocator(allocator)
    {
    _modeStrings[Shift::PropertyInstanceInformation::Internal] = "internal";
    _modeStrings[Shift::PropertyInstanceInformation::InputOutput] = "inputoutput";
    _modeStrings[Shift::PropertyInstanceInformation::InternalInput] = "internalinput";
    _modeStrings[Shift::PropertyInstanceInformation::Input] = "input";
    _modeStrings[Shift::PropertyInstanceInformation::Output] = "output";
    _modeStrings[Shift::PropertyInstanceInformation::Computed] = "computed";
    _modeStrings[Shift::PropertyInstanceInformation::InternalComputed] = "internalcomputed";
    _modeStrings[Shift::PropertyInstanceInformation::UserSettable] = "usersettable";
    }

  Eks::Vector<Module *> modules;
  Eks::Vector<PropertyInformation *> types;
  Eks::Vector<TypeRegistry::Observer *> observers;
  Eks::BucketAllocator bucketAllocator;

  enum
    {
    ModeCount = Shift::PropertyInstanceInformation::NumberOfModes
    };
  Eks::String _modeStrings[ModeCount];

  typedef std::pair<const PropertyInformation *, xuint32> InterfaceKey;
  Eks::UnorderedMap<InterfaceKey, const InterfaceBaseFactory*> interfaces;

  Eks::AllocatorBase *baseAllocator;
  };

static Eks::UniquePointer<TypeData> _internalTypes;

TypeRegistry::TypeRegistry(Eks::AllocatorBase *baseAllocator)
  {
  _internalTypes = baseAllocator->createUnique<TypeData>(baseAllocator);

  installModule(Shift::shiftModule());
  }

TypeRegistry::~TypeRegistry()
  {
  for(xsize i = _internalTypes->modules.size()-1; i != Eks::maxFor(i); --i)
    {
    uninstallModule(*_internalTypes->modules[i]);
    }

  xAssert(_internalTypes->modules.isEmpty());
  xAssert(_internalTypes->types.isEmpty());
  xAssert(_internalTypes->interfaces.empty());

  _internalTypes = nullptr;
  }

class RegistryModuleBuilder : public ModuleBuilder
  {
  void addType(PropertyInformation *t) X_OVERRIDE
    {
    internalAddType(t);
    xForeach(TypeRegistry::Observer *o, _internalTypes->observers)
      {
      o->typeAdded(t);
      }
    }

  void removeType(PropertyInformation *t) X_OVERRIDE
    {
    xAssert(TypeRegistry::findType(t->typeName()));

    if(_internalTypes->types.contains(t))
      {
      _internalTypes->types.removeAll(t);
      }

    xForeach(TypeRegistry::Observer *o, _internalTypes->observers)
      {
      o->typeRemoved(t);
      }

    xAssert(!TypeRegistry::findType(t->typeName()));
    }

  void addInterfaceFactory(const PropertyInformation *info, const InterfaceBaseFactory *factory) X_OVERRIDE
    {
    _internalTypes->interfaces[TypeData::InterfaceKey(info, factory->interfaceTypeId())] = factory;
    }

  void removeInterfaceFactory(const PropertyInformation *info, const InterfaceBaseFactory *factory) X_OVERRIDE
    {
    _internalTypes->interfaces.erase(TypeData::InterfaceKey(info, factory->interfaceTypeId()));
    }

  void internalAddType(PropertyInformation *t)
    {
    xAssert(t);
    xAssert(!TypeRegistry::findType(t->typeName()));
    if (!_internalTypes->types.contains(t))
      {
      _internalTypes->types << t;
      }

    xAssert(TypeRegistry::findType(t->typeName()));
    }

  };

void TypeRegistry::installModule(Module &module)
  {
  RegistryModuleBuilder builder;
  _internalTypes->modules << &module;

  module.install(&builder, _internalTypes->baseAllocator);
  }

void TypeRegistry::uninstallModule(Module &module)
  {
  RegistryModuleBuilder builder;

  module.uninstall(&builder, _internalTypes->baseAllocator);

  _internalTypes->modules.removeAll(&module);
  }

Eks::AllocatorBase *TypeRegistry::persistentBlockAllocator()
  {
  xAssert(_internalTypes);
  return &_internalTypes->bucketAllocator;
  }

Eks::AllocatorBase *TypeRegistry::generalPurposeAllocator()
  {
  xAssert(_internalTypes);
  xAssert(_internalTypes->baseAllocator);
  return _internalTypes->baseAllocator;
  }

Eks::AllocatorBase *TypeRegistry::interfaceAllocator()
  {
  return generalPurposeAllocator();
  }

Eks::TemporaryAllocatorCore *TypeRegistry::temporaryAllocator()
  {
  return Eks::Core::temporaryAllocator();
  }

const Eks::Vector<const PropertyInformation *> &TypeRegistry::types()
  {
  return reinterpret_cast<Eks::Vector<const PropertyInformation *> &>(_internalTypes->types);
  }

void TypeRegistry::addTypeObserver(Observer *o)
  {
  _internalTypes->observers << o;
  }

void TypeRegistry::removeTypeObserver(Observer *o)
  {
  _internalTypes->observers.removeAll(o);
  }

const PropertyInformation *TypeRegistry::findType(const NameArg &in)
  {
  SProfileFunction
  xForeach(const PropertyInformation *info, _internalTypes->types)
    {
    if(in == info->typeName())
      {
      return info;
      }
    }
  return 0;
  }

const Eks::String &TypeRegistry::getModeString(xsize mode)
  {
  xAssert(mode < TypeData::ModeCount);
  return _internalTypes->_modeStrings[mode];
  }

xsize TypeRegistry::getModeFromString(const Eks::String &mode)
  {
  for(xsize i = 0; i < TypeData::ModeCount; ++i)
    {
    if(_internalTypes->_modeStrings[i] == mode)
      {
      return i;
      }
    }

  return PropertyInstanceInformation::Default;
  }

const InterfaceBaseFactory *TypeRegistry::interfaceFactory(
    const PropertyInformation *info,
    xuint32 typeId)
  {
  return _internalTypes->interfaces.value(TypeData::InterfaceKey(info, typeId), 0);
  }

}


S_IMPLEMENT_MODULE_WITH_INTERFACES(Shift)
  {
  using namespace Shift;
#if X_QT_INTEROP
  module.addStaticInterface<Entity, SBasicPositionInterface>();
  module.addStaticInterface<Property, SBasicColourInterface>();
#endif

  module.addInheritedInterface<Database, Handler>();
  }
