#include "XBucketAllocator"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertygroup.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/Changes/sobserver.h"

namespace Shift
{

struct TypeData
  {
  Eks::Vector<const PropertyGroup *> groups;
  Eks::Vector<const PropertyInformation *> types;
  Eks::Vector<TypeRegistry::Observer *> observers;

  Eks::AllocatorBase *allocator;
  };

static TypeData *_internalTypes = 0;;

TypeRegistry::TypeRegistry()
  {
  }

void TypeRegistry::initiate()
  {
  XScript::Engine::initiate(false);

  _internalTypes = new TypeData();

  _internalTypes->allocator = new Eks::BucketAllocator();

  addPropertyGroup(Shift::propertyGroup());

  XScript::Interface<TreeObserver> *treeObs = XScript::Interface<TreeObserver>::create("_TreeObserver");
  treeObs->seal();
  }

void TypeRegistry::terminate()
{
  // script engine needs to access type info.
  XScript::Engine::terminate();

  delete _internalTypes->allocator;
  _internalTypes->allocator = 0;

  delete _internalTypes;
  }

Eks::AllocatorBase *TypeRegistry::allocator()
  {
  xAssert(_internalTypes->allocator);
  return _internalTypes->allocator;
  }

void TypeRegistry::addPropertyGroup(PropertyGroup &g)
  {
  _internalTypes->groups << &g;
  g.bootstrap();
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
