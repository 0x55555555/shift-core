#include "shift/TypeInformation/spropertyinformationhelpers.h"

namespace Shift
{

PropertyInformationChildrenCreator::PropertyInformationChildrenCreator(
    PropertyInformation *i,
    const PropertyInformationCreateData &data)
  : _information(i),
    _data(data),
    _temporaryAllocator(TypeRegistry::temporaryAllocator())
  {
  }

PropertyInformationChildrenCreator::~PropertyInformationChildrenCreator()
  {
  xsize ownedChildCount = _properties.size();

  xsize childCount = ownedChildCount + _information->parentTypeInformation()->childCount();

  EmbeddedPropertyInstanceInformation **children =
    (EmbeddedPropertyInstanceInformation **)
      _data.allocator->alloc(sizeof(EmbeddedPropertyInstanceInformation *) * childCount);

  xCompileTimeAssert(sizeof(_information->childCount()) == sizeof(xuint8));
  xAssert(childCount < X_UINT8_SENTINEL)

  // copy into children.
  // unique names.

  _information->setChildData(children);
  _information->setChildCount(childCount);
  _information->setOwnedChildCount(ownedChildCount);
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


xsize *PropertyInformationChildrenCreator::createAffects(
    const EmbeddedPropertyInstanceInformation **info,
    xsize size)
  {
  return createAffects(_data.allocator, info, size);
  }

EmbeddedPropertyInstanceInformation *PropertyInformationChildrenCreator::add(
    const PropertyInformation *newChildType,
    const PropertyNameArg &name)
  {
  xsize backwardsOffset = 0;
  PropertyInformation *allocatable = _information->findAllocatableBase(backwardsOffset);
  xAssert(allocatable);

  // size of the old type
  xsize oldAlignedSize = Eks::roundToAlignment(allocatable->size());

  // the actual object will start at this offset before the type
  xptrdiff firstFreeByte = oldAlignedSize - allocatable->propertyDataOffset();
  xAssert(firstFreeByte > 0);

  // location of the Property Data
  xsize propertyDataLocation = firstFreeByte + newChildType->propertyDataOffset();

  xsize finalSize = propertyDataLocation + newChildType->size();

  allocatable->setSize(finalSize);

  xAssert(propertyDataLocation > backwardsOffset);
  xsize location = propertyDataLocation - backwardsOffset;

  EmbeddedPropertyInstanceInformation *def = add(newChildType, location, name, true);

#ifdef X_DEBUG
  const Property *prop = def->locateProperty((const PropertyContainer*)0);
  xAssert((backwardsOffset + (xsize)prop) == propertyDataLocation);
#endif

  return def;
  }

EmbeddedPropertyInstanceInformation *PropertyInformationChildrenCreator::add(
    const PropertyInformation *newChildType,
    xsize location,
    const PropertyNameArg &name,
    bool notClassMember)
  {
  xAssert(newChildType);

  EmbeddedPropertyInstanceInformation* def =
      EmbeddedPropertyInstanceInformation::allocate(
        _data.allocator, newChildType->embeddedInstanceInformationSize());

  newChildType->functions().createEmbeddedInstanceInformation(def);

  def->initiate(newChildType, name, _properties.size(), location);

  def->setHoldingTypeInformation(_information);
  def->setIsExtraClassMember(notClassMember);

  _properties.pushBack(def);

  return def;
  }

}
