#ifndef SPROPERTYINFORMATIONAPIUTILITIES_H
#define SPROPERTYINFORMATIONAPIUTILITIES_H

#include "XInterface.h"
#include "XTemporaryAllocator"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/spropertyinstanceinformation.h"
#include "shift/TypeInformation/spropertygroup.h"
#include "shift/TypeInformation/spropertytraits.h"
#include "shift/Properties/sproperty.h"
#include "shift/Properties/sproperty.inl"
#include "shift/Utilities/sresourcedescription.h"

namespace Shift
{

template <typename T> class PropertyInformationTyped;

#define S_IMPLEMENT_PROPERTY(myName, grp) \
  static Shift::PropertyGroup::Information _##myName##StaticTypeInformation = \
    grp :: propertyGroup().registerPropertyInformation( \
    &_##myName##StaticTypeInformation, myName::bootstrapStaticTypeInformation); \
  const Shift::PropertyInformation *myName::staticTypeInformation() { return _##myName##StaticTypeInformation.information; } \
  const Shift::PropertyInformation *myName::bootstrapStaticTypeInformation(Eks::AllocatorBase *allocator) \
  { Shift::PropertyInformationTyped<myName>::bootstrapTypeInformation(&_##myName##StaticTypeInformation.information, \
  #myName, myName::ParentType::bootstrapStaticTypeInformation(allocator), allocator); return staticTypeInformation(); }

#define S_IMPLEMENT_ABSTRACT_PROPERTY(myName, grp) \
  S_IMPLEMENT_PROPERTY(myName, grp)

namespace
{
template <typename PropType> struct ApiHelper
  {
public:
  static void create(PropertyInformation *info)
    {
    const XScript::Interface<typename PropType::ParentType> *parentTempl =
        static_cast<XScript::Interface<typename PropType::ParentType>*>(PropType::ParentType::staticTypeInformation()->apiInterface());
    const XScript::Interface<Property> *baseTempl =
        static_cast<XScript::Interface<Property>*>(Property::staticTypeInformation()->apiInterface());

    XScript::Interface<PropType> *templ = XScript::Interface<PropType>::createWithParent(info->typeName(), parentTempl, baseTempl);
    info->setApiInterface(templ);
    }
  };

template <> struct ApiHelper<Property>
  {
public:
  static void create(PropertyInformation *info)
    {
    XScript::Interface<Property> *templ = XScript::Interface<Property>::create(info->typeName().data());
    info->setApiInterface(templ);
    }
  };

template <typename T, void FUNC( const PropertyInstanceInformation *, T * )> struct ComputeHelper
  {
  static void compute( const PropertyInstanceInformation *c, Property *prop)
    {
    T* t = prop->uncheckedCastTo<T>();
    xAssert(t);
    FUNC(c, t);
    }
  };

template <typename T, void FUNC( T * )> struct ComputeNoInstanceInformationHelper
  {
  static void compute( const PropertyInstanceInformation *, Property *prop)
    {
    T* t = prop->uncheckedCastTo<T>();
    xAssert(t);
    FUNC(t);
    }
  };
}

template <typename PropType, typename InstanceType> class PropertyInstanceInformationTyped
    : public InstanceType::EmbeddedInstanceInformation
  {
public:
  using InstanceType::EmbeddedInstanceInformation::setCompute;
  typedef typename InstanceType::EmbeddedInstanceInformation::ComputeFunction Function;

  template <void FUNC(PropType * )>
      void setCompute()
    {
    Function t = (Function)ComputeNoInstanceInformationHelper<PropType, FUNC>::compute;

    setCompute(t);
    }

  template <void FUNC( const PropertyInstanceInformation *, PropType * )>
      void setComputeWithInstanceInformation()
    {
    Function t = (Function)ComputeHelper<PropType, FUNC>::compute;

    setCompute(t);
    }
  };

class SHIFT_EXPORT PropertyInformationChildrenCreator
  {
public:
  EmbeddedPropertyInstanceInformation *add(
      const PropertyInformation *newChildType,
      xsize location,
      const PropertyNameArg &name,
      bool notClassMember);

  EmbeddedPropertyInstanceInformation *add(
      const PropertyInformation *newChildType,
      const PropertyNameArg &name);

  xsize *createAffects(
      const EmbeddedPropertyInstanceInformation **info,
      xsize size);

  static xsize *createAffects(
      Eks::AllocatorBase *allocator,
      const EmbeddedPropertyInstanceInformation **info,
      xsize size);

  const EmbeddedPropertyInstanceInformation *child(xsize location) const;
  EmbeddedPropertyInstanceInformation *overrideChild(xsize location);

  ~PropertyInformationChildrenCreator();

protected:
  PropertyInformationChildrenCreator(
    PropertyInformation *,
    const PropertyInformationCreateData& data);


  PropertyInformation *_information;
  const PropertyInformationCreateData &_data;

private:
  X_DISABLE_COPY(PropertyInformationChildrenCreator);
  
  Eks::TemporaryAllocator _temporaryAllocator;
  Eks::Vector<EmbeddedPropertyInstanceInformation *> _properties;
  };

template <typename PropType> class PropertyInformationChildrenCreatorTyped
    : PropertyInformationChildrenCreator
  {
public:
  template <typename U, typename AncestorPropType>
      PropertyInstanceInformationTyped<PropType, U> *add(
          U AncestorPropType::* ptr,
          const PropertyNameArg &name)
    {
    // this isnt always true, but normally is.
    // when adding extended child properties things get weird.
    //
    //typedef std::is_base_of<AncestorPropType, PropType> Inherits;
    //xCompileTimeAssert(Inherits::value == true);

    xptrdiff location = findLocation(ptr);

    xsize offset = 0;
    _information->findAllocatableBase(offset);
    location -= offset;

    return add<U>(location, name);
    }

  // add a dynamic child, ie it is embedded in the container when created,
  // but not accessible via a member.
  // this will fail and go crazy if you try to aggregate an entity with dynamic members...
  // i should fix this...
  template <typename T>
      PropertyInstanceInformationTyped<PropType, T> *add(
          const PropertyNameArg &name)
    {
    const PropertyInformation *newChildType = T::bootstrapStaticTypeInformation();

    EmbeddedPropertyInstanceInformation *inst =
        PropertyInformationChildrenCreator::add(data, newChildType, name);

    return static_cast<PropertyInstanceInformationTyped<PropType, T> *>(inst);
    }

  template <typename T>
      PropertyInstanceInformationTyped<PropType, T> *add(
          xsize location,
          const PropertyNameArg &name)
    {
    const PropertyInformation *newChildType = T::bootstrapStaticTypeInformation(_data.allocator);

    EmbeddedPropertyInstanceInformation *inst =
      PropertyInformationChildrenCreator::add(newChildType, location, name, false);

    return static_cast<PropertyInstanceInformationTyped<PropType, T> *>(inst);
    }

  template <typename T> xsize *createAffects(T a, xsize i)
    {
    return PropertyInformationChildrenCreator::createAffects(
          (const EmbeddedPropertyInstanceInformation**)a,
          i);
    }

  template <typename U, typename AncestorPropType> static xsize findLocation(U AncestorPropType::* ptr)
    {
    // avoid special casing for zero static cast
    AncestorPropType *u = reinterpret_cast<AncestorPropType*>(1);
    PropertyContainer *container = static_cast<PropertyContainer *>(u);
    U *offset = &(u->*ptr);

    Property *propOffset = offset;

    // one added earlier is cancelled out because the 1 is counted in both offset and container
    xptrdiff location = reinterpret_cast<xsize>(propOffset) - reinterpret_cast<xsize>(container);
    xAssert(location > 0);

    return (xsize)location;
    }

  template <typename U, typename PropTypeAncestor>
      PropertyInstanceInformationTyped<PropType, U> *overrideChild(U PropTypeAncestor::* ptr)
    {
    xsize location = PropertyInformationChildrenCreatorTyped<PropType>::findLocation(ptr);

    return static_cast<PropertyInstanceInformationTyped<PropType, U>*>(PropertyInformationChildrenCreator::overrideChild(location));
    }

  template <typename U, typename PropTypeAncestor>
  const PropertyInstanceInformationTyped<PropType, U> *child(U PropTypeAncestor::* ptr) const
    {
    xsize location = PropertyInformationChildrenCreatorTyped<PropType>::findLocation(ptr);

    xsize offset = 0;
    const PropertyInformation* allocatable = _information->findAllocatableBase(offset);
    (void)allocatable;
    location -= offset;

    return static_cast<const PropertyInstanceInformationTyped<PropType, U>*>(PropertyInformationChildrenCreator::child(location));
    }

private:
  friend class PropertyInformationTyped<PropType>;

  PropertyInformationChildrenCreatorTyped(
    PropertyInformationTyped<PropType> *i,
    const PropertyInformationCreateData& data)
      : PropertyInformationChildrenCreator(i, data)
    {
    }
  };

template <typename PropType> class PropertyInformationTyped : public PropertyInformation
  {
public:
  static void bootstrapTypeInformation(PropertyInformation **info,
                               const char *name,
                               const PropertyInformation *parent,
                               Eks::AllocatorBase *allocator)
    {
    if(!*info)
      {
      *info = createTypeInformation(name, parent, allocator);
      }
    }

  PropertyInformationChildrenCreatorTyped<PropType> createChildrenBlock(
      const PropertyInformationCreateData &d)
    {
    return PropertyInformationChildrenCreatorTyped<PropType>(this, d);
    }

  XScript::Interface<PropType> *apiInterface()
    {
    return static_cast<XScript::Interface<PropType>*>(PropertyInformation::apiInterface());
    }

  const XScript::Interface<PropType> *apiInterface() const
    {
    return static_cast<const XScript::Interface<PropType>*>(PropertyInformation::apiInterface());
    }

  static PropertyInformation *createTypeInformation(const char *name, const PropertyInformation *parentType, Eks::AllocatorBase *allocator)
    {
    typedef void (*FnType)(Eks::AllocatorBase *, PropertyInformationTyped<PropType> *, const char *);
    FnType fn = PropertyInformationTyped<PropType>::initiate;

    typedef void (*BaseFnType)(Eks::AllocatorBase *, PropertyInformation *, const char *);
    BaseFnType bFn = (BaseFnType)fn;

    return createTypeInformationInternal(name, parentType, bFn, allocator);
    }

  template <typename PropTypeIn, typename InstanceTypeIn>
      PropertyInformationTyped<InstanceTypeIn> *
          extendContainedProperty(
          const PropertyInformationCreateData &data,
          PropertyInstanceInformationTyped<PropTypeIn, InstanceTypeIn> *inst)
    {
    PropertyInformation *info = PropertyInformation::extendContainedProperty(data, inst);

    return static_cast<PropertyInformationTyped<InstanceTypeIn>*>(info);
    }

#ifdef S_PROPERTY_USER_DATA
  template <typename T> void addAddonInterface() const
    {
    class AddonInterface : public InterfaceBaseFactory
      {
      S_INTERFACE_FACTORY_TYPE(T)
      AddonInterface() : InterfaceBaseFactory(true) { }
      virtual InterfaceBase *classInterface(Property *prop)
        {
        Property::UserData *userData = prop->firstUserData();
        while(userData)
          {
          if(userData->userDataTypeId() == SUserDataTypes::InterfaceUserDataType)
            {
            InterfaceBase *interfaceBase = static_cast<InterfaceBase*>(userData);
            if(interfaceBase->interfaceTypeId() == T::InterfaceType::InterfaceTypeId)
              {
              return interfaceBase;
              }
            }
          userData = userData->next();
          }

        // none found, create one and add it.
        xAssertFail(); // todo, dont new this
        T* newInterface = new T(prop);
        prop->addUserData(newInterface);
        return newInterface;
        }
      };

    xAssertFail(); // todo, dont new this
    addInterfaceFactory(new AddonInterface);
    }
#endif

private:
  static void initiate(Eks::AllocatorBase *allocator, PropertyInformationTyped<PropType> *info, const char *typeName)
    {
    PropertyTraits::build<PropType>(info->functions());

    info->setChildData(0);
    info->setChildCount(0);

    info->setVersion(PropType::Version);
    info->setSize(sizeof(PropType));
    info->setDynamicInstanceInformationFormat(
      Eks::ResourceDescriptionTypeHelper<typename PropType::DynamicInstanceInformation>::createFor());
    info->setEmbeddedInstanceInformationFormat(
      Eks::ResourceDescriptionTypeHelper<typename PropType::EmbeddedInstanceInformation>::createFor());

    PropType *offset = (PropType*)1;
    Property *propertyData = offset;
    xsize propertyDataOffset = (xsize)propertyData - 1;

    info->setPropertyDataOffset(propertyDataOffset);

    info->setExtendedParent(0);

    info->setTypeName(typeName);

    info->setInstances(0);

    ApiHelper<PropType>::create(info);

    PropertyInformationCreateData data(allocator);
    data.registerAttributes = true;
    data.registerInterfaces = true;
    PropType::createTypeInformation(info, data);
    }
  };

}

namespace XScript
{
namespace Convert
{
namespace internal
{
template <> struct SHIFT_EXPORT JSToNative<Shift::PropertyInformation>
  {
  typedef const Shift::PropertyInformation *ResultType;

  ResultType operator()(Value const &h) const;
  };

template <> struct SHIFT_EXPORT NativeToJS<Shift::PropertyInformation>
  {
  Value operator()(const Shift::PropertyInformation *x) const;
  Value operator()(const Shift::PropertyInformation &x) const;
  };
}
}
}

#endif // SPROPERTYINFORMATIONAPIUTILITIES_H
