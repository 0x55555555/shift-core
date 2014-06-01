#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/Properties/scontainer.h"

namespace Shift
{

PropertyInformation *PropertyInformation::allocate(Eks::AllocatorBase *allocator)
  {
  return allocator->create<PropertyInformation>();
  }

void PropertyInformation::destroyChildren(PropertyInformation *d, Eks::AllocatorBase *allocator)
  {
  for(xuint8 i = 0; i < d->childCount(); ++i)
    {
    EmbeddedPropertyInstanceInformation *inst =
        const_cast<EmbeddedPropertyInstanceInformation*>(d->_childData[i]);

    EmbeddedPropertyInstanceInformation::destroy(allocator, inst);
    }

  allocator->free(d->_childData);
  d->_childData = 0;
  d->_childCount = 0;
  }

void PropertyInformation::destroy(PropertyInformation *d, Eks::AllocatorBase *allocator)
  {
  allocator->destroy(d);
  }

void PropertyInformation::initiate(PropertyInformation *info, const PropertyInformation *from)
  {
  // update template constructor too
  info->setFunctions(from->functions());

  info->setFormat(from->format());
  info->setPropertyDataOffset(from->propertyDataOffset());
  info->setDynamicInstanceInformationFormat(from->dynamicInstanceInformationFormat());
  info->setEmbeddedInstanceInformationFormat(from->embeddedInstanceInformationFormat());

  info->_instances = 0;
  info->_extendedParent = 0;

  info->_parentTypeInformation = from->_parentTypeInformation;

  info->_typeName = from->typeName();
  }

bool PropertyInformation::hasIndexedChildren() const
  {
  return (dynamicChildMode() & IndexedChildren) != 0;
  }

PropertyInformation *PropertyInformation::derive(
    Module &module,
    const PropertyInformation *from,
    Eks::AllocatorBase *allocator,
    bool addChildren)
  {
  PropertyInformationCreateData data(module, allocator);

  PropertyInformation *copy = PropertyInformation::allocate(allocator);

  PropertyInformation::initiate(copy, from);

  if (addChildren)
    {
    xsize childCount = from->childCount();
    xsize childByteCount = childCount * sizeof(EmbeddedPropertyInstanceInformation *);

    const EmbeddedPropertyInstanceInformation **children =
      (const EmbeddedPropertyInstanceInformation **)
        allocator->alloc(childByteCount);

    for(xsize i = 0, s = from->childCount(); i < s; ++i)
      {
      EmbeddedPropertyInstanceInformation* inst =
          const_cast<EmbeddedPropertyInstanceInformation*>(from->childFromIndex(i));

      auto ref = inst->referenceCount();
      xAssert(ref < Eks::maxFor(ref));
      ++ref;

      inst->setReferenceCount(ref);

      children[i] = inst;
      }

    xAssert(childCount < std::numeric_limits<xuint8>::max());
    copy->setChildCount((xuint8)childCount);
    copy->setChildData(children);
    }
  else
    {
    copy->setChildCount(0);
    copy->setChildData(0);
    }

  copy->_parentTypeInformation = from;

  xAssert(copy);
  return copy;
  }

PropertyInformation *PropertyInformation::extendContainedProperty(
    const PropertyInformationCreateData &data,
    EmbeddedPropertyInstanceInformation *inst)
  {
  const PropertyInformation *oldInst = inst->childInformation();
  PropertyInformation *info = PropertyInformation::derive(data.module, oldInst, data.allocator, false);

  info->setExtendedParent(inst);
  inst->setChildInformation(info);

  return info;
  }

PropertyInformation *PropertyInformation::createTypeInformationInternal(
    Module &module,
    const char *name,
    const PropertyInformation *parentType,
    void (init)(Module &, Eks::AllocatorBase *, PropertyInformation *, const char *),
    Eks::AllocatorBase *allocator)
  {
  SProfileScopedBlock("Initiate information")

  PropertyInformation *createdInfo = PropertyInformation::allocate(allocator);
  xAssert(createdInfo);

  createdInfo->setParentTypeInformation(parentType);

  init(module, allocator, createdInfo, name);

  xAssert(!parentType || createdInfo->childCount() >= parentType->childCount());
  return createdInfo;
  }

const EmbeddedPropertyInstanceInformation *PropertyInformation::child(xsize location) const
  {
  xForeach(auto i, childWalker())
    {
    if(i->location() == location)
      {
      return i;
      }
    }

  xAssertFail();
  return 0;
  }

const EmbeddedPropertyInstanceInformation *PropertyInformation::childFromName(const NameArg &in) const
  {
  xForeach(auto i, childWalker())
    {
    if(in == i->name())
      {
      return i;
      }
    }
  return 0;
  }

const PropertyInformation *PropertyInformation::findAllocatableBase(Eks::RelativeMemoryResource &offset) const
  {
  offset = Eks::RelativeMemoryResource();

  const EmbeddedPropertyInstanceInformation *allocateOnInfo = extendedParent();
  if(!allocateOnInfo)
    {
    return this;
    }

  const PropertyInformation *allocateOn = this;
  while(allocateOnInfo)
    {
    offset.increment(allocateOnInfo->location());

    const PropertyInformation *holdingInfo = allocateOnInfo->holdingTypeInformation();
    if(!holdingInfo)
      {
      return allocateOn;
      }

    allocateOn = holdingInfo;
    allocateOnInfo = allocateOn->extendedParent();
    }

  return 0;
  }

PropertyInformation *PropertyInformation::findAllocatableBase(Eks::RelativeMemoryResource &offset)
  {
  offset = Eks::RelativeMemoryResource();

  const EmbeddedPropertyInstanceInformation *allocateOnInfo = extendedParent();
  if(!allocateOnInfo)
    {
    return this;
    }

  PropertyInformation *allocateOn = this;
  while(allocateOnInfo)
    {
    offset.increment(allocateOnInfo->location());

    PropertyInformation *holdingInfo = allocateOnInfo->holdingTypeInformation();
    if(!holdingInfo)
      {
      return allocateOn;
      }

    allocateOn = holdingInfo;
    allocateOnInfo = allocateOn->extendedParent();
    }

  return allocateOn;
  }

bool PropertyInformation::inheritsFromType(const PropertyInformation *match) const
  {
  const PropertyInformation *type = this;
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

void PropertyInformation::reference() const
  {
  ++((PropertyInformation*)this)->_instances;
  }

void PropertyInformation::dereference() const
  {
  --((PropertyInformation*)this)->_instances;
  }

}
