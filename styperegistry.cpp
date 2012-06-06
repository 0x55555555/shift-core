#include "styperegistry.h"
#include "XBucketAllocator"
#include "spropertygroup.h"
#include "spropertyinformation.h"
#include "sobserver.h"

struct TypeData
  {
  QVector <const SPropertyGroup *> groups;
  QVector <const SPropertyInformation *> types;
  QList <STypeRegistry::Observer *> observers;

  XAllocatorBase *allocator;
  };

static TypeData *_internalTypes = 0;;

STypeRegistry::STypeRegistry()
  {
  }

void STypeRegistry::initiate()
  {
  XScriptEngine::initiate();

  _internalTypes = new TypeData();

  _internalTypes->allocator = new XBucketAllocator();

  addPropertyGroup(Shift::propertyGroup());

  XInterface<STreeObserver> *treeObs = XInterface<STreeObserver>::create("_STreeObserver");
  treeObs->seal();
  }

void STypeRegistry::terminate()
{
  // script engine needs to access type info.
  XScriptEngine::terminate();

  delete _internalTypes->allocator;
  _internalTypes->allocator = 0;

  delete _internalTypes;
  }

XAllocatorBase *STypeRegistry::allocator()
  {
  xAssert(_internalTypes->allocator);
  return _internalTypes->allocator;
  }

void STypeRegistry::addPropertyGroup(SPropertyGroup &g)
  {
  _internalTypes->groups << &g;
  g.bootstrap();
  }

const QVector <const SPropertyGroup *> &STypeRegistry::groups()
  {
  return _internalTypes->groups;
  }

const QVector <const SPropertyInformation *> &STypeRegistry::types()
  {
  return _internalTypes->types;
  }

void STypeRegistry::addType(const SPropertyInformation *t)
  {
  internalAddType(t);
  foreach(Observer *o, _internalTypes->observers)
    {
    o->typeAdded(t);
    }
  }

void STypeRegistry::addTypeObserver(Observer *o)
  {
  _internalTypes->observers << o;
  }

void STypeRegistry::removeTypeObserver(Observer *o)
  {
  _internalTypes->observers.removeAll(o);
  }

void STypeRegistry::internalAddType(const SPropertyInformation *t)
  {
  xAssert(t);
  xAssert(!findType(t->typeName()));
  if(!_internalTypes->types.contains(t))
    {
    _internalTypes->types << t;
    }
  }

const SPropertyInformation *STypeRegistry::findType(const QString &in)
  {
  SProfileFunction
  foreach(const SPropertyInformation *info, _internalTypes->types)
    {
    if(info->typeName() == in)
      {
      return info;
      }
    }
  return 0;
  }

namespace Shift
{
SPropertyGroup &propertyGroup()
  {
  static SPropertyGroup grp;
  return grp;
  }
}
