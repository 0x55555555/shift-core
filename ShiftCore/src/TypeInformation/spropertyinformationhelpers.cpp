#include "shift/TypeInformation/spropertyinformationhelpers.h"

namespace Shift
{

PropertyInformationChildrenCreator::PropertyInformationChildrenCreator(
    PropertyInformation *i,
    const PropertyInformationCreateData &data)
  : _information(i),
    _data(data),
    _temporaryAllocator(TypeRegistry::temporaryAllocator()),
    _properties(&_temporaryAllocator)
  {
  const PropertyInformation *parent = i->parentTypeInformation();
  _properties.resize(parent->childCount(), 0);
  }

PropertyInformationChildrenCreator::~PropertyInformationChildrenCreator()
  {
  const PropertyInformation *parent = _information->parentTypeInformation();

  xsize childCount = _properties.size();

  xAssert(childCount < X_UINT8_SENTINEL)
  xAssert(!_information->childData());
  _information->setChildCount((xuint8)childCount);
  if(childCount == 0)
    {
    _information->setChildData(0);
    }

  const EmbeddedPropertyInstanceInformation **children =
    (const EmbeddedPropertyInstanceInformation **)
      _data.allocator->alloc(sizeof(EmbeddedPropertyInstanceInformation *) * childCount);

  xCompileTimeAssert(sizeof(_information->childCount()) == sizeof(xuint8));
  xAssert(childCount < X_UINT8_SENTINEL);

  Eks::UnorderedMap<Name, bool> names(&_temporaryAllocator);

  // copy into children, check for unique names
  // also adjust ref counts
  for(xsize i = 0, s = _properties.size(); i < s; ++i)
    {
    EmbeddedPropertyInstanceInformation *inst = _properties[i];
    if(!inst)
      {
      inst = const_cast<EmbeddedPropertyInstanceInformation*>(parent->childFromIndex(i));
      }
    xAssert(!names.contains(inst->name()));

    const Name &name = inst->name();

    names.insert(name, true);

    auto ref = inst->referenceCount();
    xAssert(ref < X_UINT8_SENTINEL);
    ++ref;

    inst->setReferenceCount(ref);

    children[i] = inst;
    }

  _information->setChildData(children);

  _properties.clear();
  _properties.squeeze();
  }

xsize *PropertyInformationChildrenCreator::createAffects(
    Eks::AllocatorBase *allocator,
    const EmbeddedPropertyInstanceInformation **info,
    xsize size)
  {
  xsize *aff = (xsize *)allocator->alloc(sizeof(xsize) * (size+1));

  for(xsize i = 0; i < size; ++i)
    {
    aff[i] = info[i]->location();
    }

  aff[size] = 0;

  return aff;
  }

const EmbeddedPropertyInstanceInformation *PropertyInformationChildrenCreator::child(xsize location) const
  {
  const PropertyInformation *parent = _information->parentTypeInformation();

  for(xsize i = 0, s = _properties.size(); i < s; ++i)
    {
    const EmbeddedPropertyInstanceInformation *inst = _properties[i];
    if(!inst)
      {
      inst = parent->childFromIndex(i);
      }

    if(inst->location() == location)
      {
      return inst;
      }
    }

  xAssertFail();
  return 0;
  }

EmbeddedPropertyInstanceInformation *PropertyInformationChildrenCreator::overrideChild(xsize location)
  {
  if(!_information->parentTypeInformation())
    {
    return 0;
    }

  const EmbeddedPropertyInstanceInformation *oldInst =
      _information->parentTypeInformation()->child(location);

  const PropertyInformation *newType = oldInst->childInformation();

  const PropertyInformationFunctions& fns = newType->functions();

  Eks::MemoryResource def =
      EmbeddedPropertyInstanceInformation::allocate(
        _data.allocator, newType->embeddedInstanceInformationFormat());

  EmbeddedPropertyInstanceInformation *inst =
    static_cast<EmbeddedPropertyInstanceInformation*>(fns.createEmbeddedInstanceInformation(def, oldInst));
  xAssert(inst == def.data());

  inst->setHoldingTypeInformation(_information);

  xAssert(!_properties[inst->index()]);
  _properties[inst->index()] = inst;

  return inst;
  }

xsize *PropertyInformationChildrenCreator::createAffects(
    const EmbeddedPropertyInstanceInformation **info,
    xsize size)
  {
  return createAffects(_data.allocator, info, size);
  }

EmbeddedPropertyInstanceInformation *PropertyInformationChildrenCreator::add(
    const PropertyInformation *newChildType,
    const NameArg &name)
  {
  Eks::RelativeMemoryResource backwardsOffset = 0;
  PropertyInformation *allocatable = _information->findAllocatableBase(backwardsOffset);
  xAssert(allocatable);

  Eks::ResourceDescription allocatableFmt = allocatable->format();
  Eks::ResourceDescription newAllocatableFmt = allocatableFmt + newChildType->format();

  allocatable->setFormat(newAllocatableFmt);

  Eks::RelativeMemoryResource allocatableRsc;

  // Find a relative resource after the allocated type.
  Eks::RelativeMemoryResource afterAllocatable;
  allocatableRsc.alignAndIncrement(allocatableFmt, &afterAllocatable);

  // Remove the property data offset from the pointer, this finds the free location for the resource
  // if the allocatable information's ATTRIBUTE pointer is at 0x0
  Eks::RelativeMemoryResource afterAllocatableAttr = afterAllocatable.decrememt(allocatable->propertyDataOffset());
  xAssert(afterAllocatableAttr.isPost());

  // find the location of the Attribute pointer relative to the allocatable base.
  Eks::RelativeMemoryResource newAttrPropertyOffset = afterAllocatableAttr.increment(newChildType->propertyDataOffset());
  xAssert(newAttrPropertyOffset > backwardsOffset);

  // Location is the byte offset from the direct parent attribute to the child attribute's address
  Eks::RelativeMemoryResource location = newAttrPropertyOffset - backwardsOffset;

  EmbeddedPropertyInstanceInformation *def = add(newChildType, location.value(), name, true);

#ifdef X_DEBUG
  const Attribute *prop = def->locate((const Container*)0);
  xAssert(backwardsOffset.increment((xsize)prop) == newAttrPropertyOffset);
#endif

  return def;
  }

EmbeddedPropertyInstanceInformation *PropertyInformationChildrenCreator::add(
    const PropertyInformation *newChildType,
    xsize location,
    const NameArg &name,
    bool notClassMember)
  {
  xAssert(newChildType);

  Eks::MemoryResource mem =
      EmbeddedPropertyInstanceInformation::allocate(
        _data.allocator, newChildType->embeddedInstanceInformationFormat());

  EmbeddedPropertyInstanceInformation *def
    = static_cast<EmbeddedPropertyInstanceInformation*>(newChildType->functions().createEmbeddedInstanceInformation(mem, 0));

  def->initiate(newChildType, name, _properties.size(), location);

  def->setHoldingTypeInformation(_information);
  def->setIsExtraClassMember(notClassMember);

  _properties.pushBack(def);

  return def;
  }

}
