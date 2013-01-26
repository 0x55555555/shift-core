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
  _properties.resize(parent->childCount());
  }

PropertyInformationChildrenCreator::~PropertyInformationChildrenCreator()
  {
  xsize ownedChildCount = _properties.size();

  const PropertyInformation *parent = _information->parentTypeInformation();

  xsize childCount = ownedChildCount + parent->childCount();

  const EmbeddedPropertyInstanceInformation **children =
    (const EmbeddedPropertyInstanceInformation **)
      _data.allocator->alloc(sizeof(EmbeddedPropertyInstanceInformation *) * childCount);

  xCompileTimeAssert(sizeof(_information->childCount()) == sizeof(xuint8));
  xAssert(childCount < X_UINT8_SENTINEL);

  Eks::UnorderedMap<PropertyName, bool> names(&_temporaryAllocator);

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
    names.insert(inst->name(), true);

    auto ref = inst->referenceCount();
    xAssert(ref < X_UINT8_SENTINEL);
    ++ref;

    inst->setReferenceCount(ref);

    children[i] = inst;
    }

  _information->setChildData(children);
  _information->setChildCount(childCount);
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

  EmbeddedPropertyInstanceInformation* def =
      EmbeddedPropertyInstanceInformation::allocate(
        _data.allocator, newType->embeddedInstanceInformationSize());

  PropertyInstanceInformation *inst = fns.createEmbeddedInstanceInformation(def, oldInst);
  xAssert(inst == def);

  def->setHoldingTypeInformation(_information);

  xAssert(!_properties[def->index()]);
  _properties[def->index()] = def;

  return def;
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

  newChildType->functions().createEmbeddedInstanceInformation(def, 0);

  def->initiate(newChildType, name, _properties.size(), location);

  def->setHoldingTypeInformation(_information);
  def->setIsExtraClassMember(notClassMember);

  _properties.pushBack(def);

  return def;
  }

}
