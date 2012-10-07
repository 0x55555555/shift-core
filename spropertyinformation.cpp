#include "spropertyinformation.h"
#include "spropertyinformationhelpers.h"
#include "spropertycontainer.h"
#include "styperegistry.h"

struct Utils
  {
  static void callCreateTypeInformationBottomUp(SPropertyInformation *i,
                                         SPropertyInformationCreateData &d,
                                         const SPropertyInformation *from)
    {
    if(from)
      {
      const SPropertyInformation *parent = from->parentTypeInformation();
      callCreateTypeInformationBottomUp(i, d, parent);

      from->functions().createTypeInformation(i, d);
      }
    }
  };


SPropertyInformation *SPropertyInformation::allocate()
  {
  xAssert(STypeRegistry::allocator());
  void *ptr = STypeRegistry::allocator()->alloc(sizeof(SPropertyInformation));

  xAssert(ptr);
  return new(ptr) SPropertyInformation;
  }

void SPropertyInformation::destroy(SPropertyInformation *d)
  {
  Q_FOREACH(SInterfaceBaseFactory *f, d->_interfaceFactories)
    {
    xAssert(f->referenceCount() > 0);
    f->setReferenceCount(f->referenceCount() - 1);
    if(f->referenceCount() == 0 && f->deleteOnNoReferences())
      {
      delete f;
      }
    }

  xForeach(auto inst, d->childWalker())
    {
    delete [] inst->affects();
    inst->~SPropertyInstanceInformation();
    SPropertyInstanceInformation::destroy(inst);
    }
  d->_firstChild = 0;
  d->_lastChild = 0;

  xAssert(STypeRegistry::allocator());
  STypeRegistry::allocator()->free(d);
  }

void SPropertyInformation::initiate(SPropertyInformation *info, const SPropertyInformation *from)
  {
  // update template constructor too
  info->setFunctions(from->functions());

  info->setVersion(from->version());

  info->setSize(from->size());
  info->setPropertyDataOffset(from->propertyDataOffset());
  info->setInstanceInformationSize(from->instanceInformationSize());

  info->_instances = 0;
  info->_extendedParent = 0;

  info->_parentTypeInformation = from->_parentTypeInformation;

  info->_typeName = from->typeName();
  }

SPropertyInformation *SPropertyInformation::derive(const SPropertyInformation *from)
  {
  xAssert(from->functions().createTypeInformation);
  SPropertyInformation *copy = SPropertyInformation::allocate();

  SPropertyInformation::initiate(copy, from);

  copy->_parentTypeInformation = from;

  SPropertyInformationCreateData data;
  data.registerAttributes = true;
  data.registerInterfaces = false;

  Utils::callCreateTypeInformationBottomUp(copy, data, from);

  copy->_apiInterface = from->_apiInterface;
  xAssert(copy->_apiInterface);

  xAssert(copy);
  return copy;
  }

SPropertyInstanceInformation *SPropertyInformation::add(const SPropertyInformation *newChildType, const QString &name)
  {
  xsize backwardsOffset = 0;
  SPropertyInformation *allocatable = findAllocatableBase(backwardsOffset);
  xAssert(allocatable);

  // size of the old type
  xsize oldAlignedSize = X_ROUND_TO_ALIGNMENT(xsize, allocatable->size());

  // the actual object will start at this offset before the type
  xptrdiff firstFreeByte = oldAlignedSize - allocatable->propertyDataOffset();
  xAssert(firstFreeByte > 0);

  // location of the SProperty Data
  xsize propertyDataLocation = firstFreeByte + newChildType->propertyDataOffset();

  xsize finalSize = propertyDataLocation + newChildType->size();

  allocatable->setSize(finalSize);

  xAssert(propertyDataLocation > backwardsOffset);
  xsize location = propertyDataLocation - backwardsOffset;

  SPropertyInstanceInformation *def = add(newChildType, location, name, true);

#ifdef X_DEBUG
  const SProperty *prop = def->locateProperty((const SPropertyContainer*)0);
  xAssert((backwardsOffset + (xsize)prop) == propertyDataLocation);
#endif

  return def;
  }

SPropertyInstanceInformation *SPropertyInformation::add(const SPropertyInformation *newChildType, xsize location, const QString &name, bool notClassMember)
  {
  xAssert(newChildType);
  xAssert(!childFromName(name));

  SPropertyInstanceInformation* def = SPropertyInstanceInformation::allocate(newChildType->instanceInformationSize());

  newChildType->functions().createInstanceInformation(def);

  def->initiate(newChildType, name, _childCount, location);

  def->setHoldingTypeInformation(this);
  def->setIsExtraClassMember(notClassMember);

  if(_lastChild)
    {
    xAssert(_lastChild->nextSibling() == 0);
    _lastChild->_nextSibling = def;
    }
  else
    {
    _firstChild = def;
    }

  _lastChild = def;

  ++_childCount;

  return def;
  }

void SPropertyInformation::addInterfaceFactoryInternal(xuint32 typeId, SInterfaceBaseFactory *factory)
  {
  xAssert(factory);
  xAssert(typeId != SInterfaceTypes::Invalid);

  _interfaceFactories.insert(typeId, factory);

  factory->setReferenceCount(factory->referenceCount() + 1);
  xAssert(interfaceFactory(typeId) == factory);
  }

const SInterfaceBaseFactory *SPropertyInformation::interfaceFactory(xuint32 type) const
  {
  const SInterfaceBaseFactory *fac = 0;
  const SPropertyInformation *info = this;
  while(!fac && info)
    {
    fac = info->_interfaceFactories.value(type, 0);
    info = info->parentTypeInformation();
    }

  return fac;
  }

SPropertyInformation *SPropertyInformation::extendContainedProperty(SPropertyInstanceInformation *inst)
  {
  const SPropertyInformation *oldInst = inst->childInformation();
  SPropertyInformation *info = SPropertyInformation::derive(oldInst);

  info->setExtendedParent(inst);
  inst->setChildInformation(info);

  return info;
  }

SPropertyInformation *SPropertyInformation::createTypeInformationInternal(const char *name,
                                                                                const SPropertyInformation *parentType,
                                                                                void (init)(SPropertyInformation *, const char *))
  {
  SProfileScopedBlock("Initiate information")

  SPropertyInformation *createdInfo = SPropertyInformation::allocate();
  xAssert(createdInfo);

  init(createdInfo, name);

  SPropertyInformationCreateData data;

  data.registerAttributes = true;
  data.registerInterfaces = false;
  Utils::callCreateTypeInformationBottomUp(createdInfo, data, parentType);

  createdInfo->setParentTypeInformation(parentType);

  data.registerAttributes = true;
  data.registerInterfaces = true;
  createdInfo->functions().createTypeInformation(createdInfo, data);

  // seal API
  createdInfo->apiInterface()->seal();

  return createdInfo;
  }

SPropertyInstanceInformation *SPropertyInformation::child(xsize location)
  {
  xForeach(auto i, childWalker())
    {
    if(i->location() == location)
      {
      return i;
      }
    }
  return 0;
  }

const SPropertyInstanceInformation *SPropertyInformation::child(xsize location) const
  {
  xForeach(auto i, childWalker())
    {
    if(i->location() == location)
      {
      return i;
      }
    }
  return 0;
  }

const SPropertyInstanceInformation *SPropertyInformation::childFromName(const QString &in) const
  {
  xForeach(auto i, childWalker())
    {
    if(i->name() == in)
      {
      return i;
      }
    }
  return 0;
  }

SPropertyInstanceInformation *SPropertyInformation::childFromName(const QString &in)
  {
  xForeach(auto i, childWalker())
    {
    if(i->name() == in)
      {
      return i;
      }
    }
  return 0;
  }

const SPropertyInformation *SPropertyInformation::findAllocatableBase(xsize &offset) const
  {
  offset = 0;

  const SPropertyInstanceInformation *allocateOnInfo = extendedParent();
  if(!allocateOnInfo)
    {
    return this;
    }

  const SPropertyInformation *allocateOn = this;
  while(allocateOnInfo)
    {
    offset += allocateOnInfo->location();

    const SPropertyInformation *holdingInfo = allocateOnInfo->holdingTypeInformation();
    if(!holdingInfo)
      {
      return allocateOn;
      }

    allocateOn = holdingInfo;
    allocateOnInfo = allocateOn->extendedParent();
    }

  return 0;
  }

SPropertyInformation *SPropertyInformation::findAllocatableBase(xsize &offset)
  {
  offset = 0;

  const SPropertyInstanceInformation *allocateOnInfo = extendedParent();
  if(!allocateOnInfo)
    {
    return this;
    }

  SPropertyInformation *allocateOn = this;
  while(allocateOnInfo)
    {
    offset += allocateOnInfo->location();

    SPropertyInformation *holdingInfo = allocateOnInfo->holdingTypeInformation();
    if(!holdingInfo)
      {
      return allocateOn;
      }

    allocateOn = holdingInfo;
    allocateOnInfo = allocateOn->extendedParent();
    }

  return allocateOn;
  }

SPropertyInformation::DataKey g_maxKey = 0;
SPropertyInformation::DataKey SPropertyInformation::newDataKey()
  {
  return g_maxKey++;
  }

void SPropertyInformation::setData(DataKey k, const QVariant &v)
  {
  xAssert(k < g_maxKey);
  _data[k].setValue(v);
  }

bool SPropertyInformation::inheritsFromType(const SPropertyInformation *match) const
  {
  const SPropertyInformation *type = this;
  while(type)
    {
    if(type == match)
      {
      return true;
      }
    type = type->parentTypeInformation();
    }
  return false;
  }

void SPropertyInformation::reference() const
  {
  ++((SPropertyInformation*)this)->_instances;
  }

void SPropertyInformation::dereference() const
  {
  --((SPropertyInformation*)this)->_instances;
  }

namespace XScript
{
namespace Convert
{
namespace internal
{
JSToNative<SPropertyInformation>::ResultType JSToNative<SPropertyInformation>::operator()(Value const &h) const
  {
  if(h.isObject())
    {
    Object obj(h);
    return (SPropertyInformation*)STypeRegistry::findType(obj.get("typeName").toString());
    }
  else
    {
    return (SPropertyInformation*)STypeRegistry::findType(h.toString());
    }
  }

Value NativeToJS<SPropertyInformation>::operator()(const SPropertyInformation &x) const
  {
  return x.typeName();
  }

Value NativeToJS<SPropertyInformation>::operator()(const SPropertyInformation *x) const
  {
  xAssert(x)
  return x->typeName();
  }
}
}
}
