#include "XBucketAllocator"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertygroup.h"
#include "shift/TypeInformation/spropertyinstanceinformation.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/Changes/sobserver.h"
#include "shift/TypeInformation/sinterfaces.h"
#include "XTemporaryAllocator"
#include "XUnorderedMap"

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
    : groups(allocator),
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

  Eks::Vector<const PropertyGroup *> groups;
  Eks::Vector<PropertyInformation *> types;
  Eks::Vector<TypeRegistry::Observer *> observers;
  Eks::BucketAllocator bucketAllocator;

  enum
    {
    ModeCount = Shift::PropertyInstanceInformation::NumberOfModes
    };
  Eks::String _modeStrings[ModeCount];

  typedef QPair<const PropertyInformation *, xuint32> InterfaceKey;
  Eks::UnorderedMap<InterfaceKey, InterfaceBaseFactory*> interfaces;

  Eks::AllocatorBase *baseAllocator;
  };

static TypeData *_internalTypes = 0;

TypeRegistry::TypeRegistry()
  {
  }

void TypeRegistry::initiate(
    Eks::AllocatorBase *baseAllocator
    )
  {
  XScript::Engine::initiate(false);

  _internalTypes = baseAllocator->create<TypeData>(baseAllocator);

  addPropertyGroup(Shift::propertyGroup());

  setupBaseInterfaces();

  XScript::Interface<TreeObserver> *treeObs = XScript::Interface<TreeObserver>::create("_TreeObserver");
  treeObs->seal();
  }

void TypeRegistry::terminate()
  {
  Eks::AllocatorBase* alloc = _internalTypes->baseAllocator;

  xForeach(PropertyInformation *info, _internalTypes->types)
    {
    PropertyInformation::destroyChildren(info, alloc);
    }

  xForeach(PropertyInformation *info, _internalTypes->types)
    {
    PropertyInformation::destroy(info, alloc);
    }
  _internalTypes->types.clear();

  xForeach(InterfaceBaseFactory *fac, _internalTypes->interfaces)
    {
    fac->~InterfaceBaseFactory();
    void *location = ((xuint8*)fac + fac->offset());
    interfaceAllocator()->free(location);
    }

  _internalTypes->baseAllocator->destroy(_internalTypes);


  // script engine needs to access type info.
  XScript::Engine::terminate();
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

void TypeRegistry::addPropertyGroup(PropertyGroup &g)
  {
  _internalTypes->groups << &g;
  g.bootstrap(_internalTypes->baseAllocator);
  }

const Eks::Vector<const PropertyGroup *> &TypeRegistry::groups()
  {
  return _internalTypes->groups;
  }

const Eks::Vector<const PropertyInformation *> &TypeRegistry::types()
  {
  return reinterpret_cast<Eks::Vector<const PropertyInformation *> &>(_internalTypes->types);
  }

void TypeRegistry::addType(PropertyInformation *t)
  {
  internalAddType(t);
  xForeach(Observer *o, _internalTypes->observers)
    {
    o->typeAdded(t);
    }
  }

void TypeRegistry::addTypeObserver(Observer *o)
  {
  _internalTypes->observers << o;
  }

void TypeRegistry::removeTypeObserver(Observer *o)
  {
  _internalTypes->observers.removeAll(o);
  }

void TypeRegistry::internalAddType(PropertyInformation *t)
  {
  xAssert(t);
  xAssert(!findType(t->typeName()));
  if(!_internalTypes->types.contains(t))
    {
    _internalTypes->types << t;
    }
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

const xsize TypeRegistry::getModeFromString(const Eks::String &mode)
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

void TypeRegistry::addInterfaceFactory(
    const PropertyInformation *info,
    xuint32 typeId,
    InterfaceBaseFactory *factory,
    xptrdiff offset)
  {
  factory->setOffset(offset);
  _internalTypes->interfaces[TypeData::InterfaceKey(info, typeId)] = factory;
  }

PropertyGroup &propertyGroup()
  {
  static PropertyGroup grp;
  return grp;
  }
}
