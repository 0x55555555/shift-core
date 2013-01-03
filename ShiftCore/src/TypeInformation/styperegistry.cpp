#include "XBucketAllocator"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertygroup.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/Changes/sobserver.h"

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
      bucketAllocator(DefaultAllocation, ExpandingAllocation, allocator),
      baseAllocator(allocator)
    {
    }

  Eks::Vector<const PropertyGroup *> groups;
  Eks::Vector<const PropertyInformation *> types;
  Eks::Vector<TypeRegistry::Observer *> observers;
  Eks::BucketAllocator bucketAllocator;

  Eks::AllocatorBase *baseAllocator;
  };

static TypeData *_internalTypes = 0;;

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

  //Entity::staticTypeInformation()->addStaticInterface(data.allocator->create<SBasicPositionInterface>());
  //Property::staticTypeInformation()->addStaticInterface(data.allocator->create<SBasicColourInterface>());
  //Database::staticTypeInformation()->addInheritedInterface<Handler>();
  //per typed pointer property info->addStaticInterface( data.allocator->create<PODPropertyVariantInterface<name, name::PODType> >()); }
  //base->addInterfaceFactoryInternal(PointerArrayConnectionInterface::InterfaceType::InterfaceTypeId, new PointerArrayConnectionInterface);

  XScript::Interface<TreeObserver> *treeObs = XScript::Interface<TreeObserver>::create("_TreeObserver");
  treeObs->seal();
  }

void TypeRegistry::terminate()
  {
  _internalTypes->baseAllocator->destroy(_internalTypes);

  // script engine needs to access type info.
  XScript::Engine::terminate();
  }

Eks::AllocatorBase *TypeRegistry::persistentBlockAllocator()
  {
  return &_internalTypes->bucketAllocator;
  }

Eks::AllocatorBase *TypeRegistry::generalPurposeAllocator()
  {
  xAssert(_internalTypes->baseAllocator);
  return _internalTypes->baseAllocator;
  }

void TypeRegistry::addPropertyGroup(PropertyGroup &g)
  {
  _internalTypes->groups << &g;
  xAssertFail();
  g.bootstrap(_internalTypes->baseAllocator);
  }

const Eks::Vector<const PropertyGroup *> &TypeRegistry::groups()
  {
  return _internalTypes->groups;
  }

const Eks::Vector<const PropertyInformation *> &TypeRegistry::types()
  {
  return _internalTypes->types;
  }

void TypeRegistry::addType(const PropertyInformation *t)
  {
  internalAddType(t);
  Q_FOREACH(Observer *o, _internalTypes->observers)
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

void TypeRegistry::internalAddType(const PropertyInformation *t)
  {
  xAssert(t);
  xAssert(!findType(t->typeName()));
  if(!_internalTypes->types.contains(t))
    {
    _internalTypes->types << t;
    }
  }

const PropertyInformation *TypeRegistry::findType(const PropertyNameArg &in)
  {
  SProfileFunction
  Q_FOREACH(const PropertyInformation *info, _internalTypes->types)
    {
    if(in == info->typeName())
      {
      return info;
      }
    }
  return 0;
  }

PropertyGroup &propertyGroup()
  {
  static PropertyGroup grp;
  return grp;
  }
}
