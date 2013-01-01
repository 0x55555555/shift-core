#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/Properties/spropertycontainer.h"

namespace Shift
{

struct Utils
  {
  static void callCreateTypeInformationBottomUp(PropertyInformation *i,
                                         PropertyInformationCreateData &d,
                                         const PropertyInformation *from)
    {
    if(from)
      {
      const PropertyInformation *parent = from->parentTypeInformation();
      callCreateTypeInformationBottomUp(i, d, parent);

      from->functions().createTypeInformation(i, d);
      }
    }
  };


PropertyInformation *PropertyInformation::allocate()
  {
  xAssert(TypeRegistry::allocator());
  void *ptr = TypeRegistry::allocator()->alloc(sizeof(PropertyInformation));

  xAssert(ptr);
  return new(ptr) PropertyInformation;
  }

void PropertyInformation::destroy(PropertyInformation *d)
  {
  Q_FOREACH(InterfaceBaseFactory *f, d->_interfaceFactories)
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
    //delete [] inst->affects();

    PropertyInformationFunctions::DestroyInstanceInformationFunction destroy =
      inst->childInformation()->functions().destroyEmbeddedInstanceInformation;

    destroy(inst);

    PropertyInstanceInformation::destroy(inst);
    }
  d->_childData = 0;
  d->_childLimit = 0;
  d->_childEnd = 0;

  //xAssert(TypeRegistry::allocator());
  //TypeRegistry::allocator()->free(d);
  }

void PropertyInformation::initiate(PropertyInformation *info, const PropertyInformation *from)
  {
  // update template constructor too
  info->setFunctions(from->functions());

  info->setVersion(from->version());

  info->setSize(from->size());
  info->setPropertyDataOffset(from->propertyDataOffset());
  info->setDynamicInstanceInformationSize(from->dynamicInstanceInformationSize());
  info->setEmbeddedInstanceInformationSize(from->embeddedInstanceInformationSize());

  info->_instances = 0;
  info->_extendedParent = 0;

  info->_parentTypeInformation = from->_parentTypeInformation;

  info->_typeName = from->typeName();
  }

PropertyInformation *PropertyInformation::derive(const PropertyInformation *from)
  {
  xAssert(from->functions().createTypeInformation);
  PropertyInformation *copy = PropertyInformation::allocate();

  PropertyInformation::initiate(copy, from);

  copy->_parentTypeInformation = from;

  PropertyInformationCreateData data;
  data.registerAttributes = true;
  data.registerInterfaces = false;

  Utils::callCreateTypeInformationBottomUp(copy, data, from);

  copy->_apiInterface = from->_apiInterface;
  xAssert(copy->_apiInterface);

  xAssert(copy);
  return copy;
  }

EmbeddedPropertyInstanceInformation *PropertyInformation::add(const PropertyInformation *newChildType, const QString &name)
  {
  xsize backwardsOffset = 0;
  PropertyInformation *allocatable = findAllocatableBase(backwardsOffset);
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

EmbeddedPropertyInstanceInformation *PropertyInformation::add(const PropertyInformation *newChildType, xsize location, const QString &name, bool notClassMember)
  {
  xAssert(newChildType);
  xAssert(!childFromName(name));
  xAssert(_childEnd < _childLimit);


  EmbeddedPropertyInstanceInformation* def = EmbeddedPropertyInstanceInformation::allocate(newChildType->embeddedInstanceInformationSize());

  newChildType->functions().createEmbeddedInstanceInformation(def);

  def->initiate(newChildType, name, childCount(), location);

  def->setHoldingTypeInformation(this);
  def->setIsExtraClassMember(notClassMember);

  *_childEnd = def;
  ++_childEnd;

  return def;
  }

void PropertyInformation::addInterfaceFactoryInternal(xuint32 typeId, InterfaceBaseFactory *factory)
  {
  xAssert(factory);
  xAssert(typeId != SInterfaceTypes::Invalid);

  _interfaceFactories.insert(typeId, factory);

  factory->setReferenceCount(factory->referenceCount() + 1);
  xAssert(interfaceFactory(typeId) == factory);
  }

const InterfaceBaseFactory *PropertyInformation::interfaceFactory(xuint32 type) const
  {
  const InterfaceBaseFactory *fac = 0;
  const PropertyInformation *info = this;
  while(!fac && info)
    {
    fac = info->_interfaceFactories.value(type, 0);
    info = info->parentTypeInformation();
    }

  return fac;
  }

PropertyInformation *PropertyInformation::extendContainedProperty(EmbeddedPropertyInstanceInformation *inst)
  {
  const PropertyInformation *oldInst = inst->childInformation();
  PropertyInformation *info = PropertyInformation::derive(oldInst);

  info->setExtendedParent(inst);
  inst->setChildInformation(info);

  return info;
  }

PropertyInformation *PropertyInformation::createTypeInformationInternal(const char *name,
                                                                                const PropertyInformation *parentType,
                                                                                void (init)(PropertyInformation *, const char *))
  {
  SProfileScopedBlock("Initiate information")

  PropertyInformation *createdInfo = PropertyInformation::allocate();
  xAssert(createdInfo);

  init(createdInfo, name);

  PropertyInformationCreateData data;

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

EmbeddedPropertyInstanceInformation *PropertyInformation::child(xsize location)
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

const EmbeddedPropertyInstanceInformation *PropertyInformation::child(xsize location) const
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

const EmbeddedPropertyInstanceInformation *PropertyInformation::childFromName(const PropertyNameArg &in) const
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

EmbeddedPropertyInstanceInformation *PropertyInformation::childFromName(const PropertyNameArg &in)
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

const PropertyInformation *PropertyInformation::findAllocatableBase(xsize &offset) const
  {
  offset = 0;

  const EmbeddedPropertyInstanceInformation *allocateOnInfo = extendedParent();
  if(!allocateOnInfo)
    {
    return this;
    }

  const PropertyInformation *allocateOn = this;
  while(allocateOnInfo)
    {
    offset += allocateOnInfo->location();

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

PropertyInformation *PropertyInformation::findAllocatableBase(xsize &offset)
  {
  offset = 0;

  const EmbeddedPropertyInstanceInformation *allocateOnInfo = extendedParent();
  if(!allocateOnInfo)
    {
    return this;
    }

  PropertyInformation *allocateOn = this;
  while(allocateOnInfo)
    {
    offset += allocateOnInfo->location();

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

PropertyInformation::DataKey g_maxKey = 0;
PropertyInformation::DataKey PropertyInformation::newDataKey()
  {
  return g_maxKey++;
  }

void PropertyInformation::setData(DataKey k, const QVariant &v)
  {
  xAssert(k < g_maxKey);
  _data[k].setValue(v);
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

namespace XScript
{
namespace Convert
{
namespace internal
{
JSToNative<Shift::PropertyInformation>::ResultType JSToNative<Shift::PropertyInformation>::operator()(Value const &h) const
  {
  if(h.isObject())
    {
    Object obj(h);
    return (Shift::PropertyInformation*)Shift::TypeRegistry::findType(obj.get("typeName").toString());
    }
  else
    {
    return (Shift::PropertyInformation*)Shift::TypeRegistry::findType(h.toString());
    }
  }

Value NativeToJS<Shift::PropertyInformation>::operator()(const Shift::PropertyInformation &x) const
  {
  return x.typeName().toQString();
  }

Value NativeToJS<Shift::PropertyInformation>::operator()(const Shift::PropertyInformation *x) const
  {
  xAssert(x)
  return x->typeName().toQString();
  }
}
}
}
