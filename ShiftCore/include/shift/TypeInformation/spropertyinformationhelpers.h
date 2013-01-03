#ifndef SPROPERTYINFORMATIONAPIUTILITIES_H
#define SPROPERTYINFORMATIONAPIUTILITIES_H

#include "XInterface.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/spropertyinstanceinformation.h"
#include "shift/TypeInformation/spropertygroup.h"
#include "shift/Properties/sproperty.h"
#include "shift/Properties/sproperty.inl"
#include "shift/Utilities/sresourcedescription.h"

namespace Shift
{

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

template<typename T, bool Abstract = T::IsAbstract> struct PropertyHelper;

template<typename T> struct PropertyHelper<T, true>
  {
  static Property *create(void *)
    {
    xAssertFail();
    return 0;
    }
  static void createInPlace(Property *)
    {
    xAssertFail();
    }
  static void destroy(Property *)
    {
    xAssertFail();
    }
  };

template<typename T> struct PropertyHelper<T, false>
  {
  static Property *create(void *ptr)
    {
    return new(ptr) T();
    }
  static void createInPlace(Property *ptr)
    {
    T* t = static_cast<T*>(ptr);
    new(t) T();
    }
  static void destroy(Property *ptr)
    {
    (void)ptr;
    ((T*)ptr)->~T();
    }
  };

template <typename T> struct InstanceInformationHelper
  {
  typedef typename T::DynamicInstanceInformation DyInst;
  typedef typename T::EmbeddedInstanceInformation StInst;
  static PropertyInstanceInformation *createDynamic(void *allocation)
    {
    return new(allocation) DyInst;
    }
  static PropertyInstanceInformation *createEmbedded(void *allocation)
    {
    return new(allocation) StInst;
    }
  static void destroyDynamic(PropertyInstanceInformation *allocation)
    {
    ((DyInst*)allocation)->~DyInst();
    }
  static void destroyEmbedded(PropertyInstanceInformation *allocation)
    {
    ((StInst*)allocation)->~StInst();
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

template <typename PropType, typename InstanceType> class PropertyInstanceInformationTyped : public InstanceType::EmbeddedInstanceInformation
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

  template <typename U, typename PropTypeAncestor>
  PropertyInstanceInformationTyped<PropType, U> *child(U PropTypeAncestor::* ptr)
    {
    xsize location = findLocation(ptr);

    return static_cast<PropertyInstanceInformationTyped<PropType, U>*>(PropertyInformation::child(location));
    }

  template <typename U, typename PropTypeAncestor>
  const PropertyInstanceInformationTyped<PropType, U> *child(U PropTypeAncestor::* ptr) const
    {
    xsize location = findLocation(ptr);

    xsize offset = 0;
    PropertyInformation* allocatable = findAllocatableBase(offset);
    location -= offset;

    return static_cast<const PropertyInstanceInformationTyped<PropType, U>*>(PropertyInformation::child(location));
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
    typedef void (*FnType)(PropertyInformation *, const char *);
    FnType fn = PropertyInformationTyped<PropType>::initiate;

    return createTypeInformationInternal(name, parentType, fn, allocator);
    }

  template <typename U, typename AncestorPropType> xsize findLocation(U AncestorPropType::* ptr)
    {
    AncestorPropType *u = reinterpret_cast<AncestorPropType*>(1); // avoid special casing for zero static cast
    PropertyContainer *container = static_cast<PropertyContainer *>(u);
    U *offset = &(u->*ptr);

    Property *propOffset = offset;

    // one added earlier is cancelled out because the 1 is counted in both offset and container
    xptrdiff location = reinterpret_cast<xsize>(propOffset) - reinterpret_cast<xsize>(container);
    xAssert(location > 0);

    return (xsize)location;
    }

  template <typename U, typename AncestorPropType>
      PropertyInstanceInformationTyped<PropType, U> *add(
          const PropertyInformationCreateData &data,
          U AncestorPropType::* ptr,
          const PropertyNameArg &name)
    {
    xptrdiff location = findLocation(ptr);

    xsize offset = 0;
    findAllocatableBase(offset);
    location -= offset;

    return add<U>(data, location, name);
    }

  // add a dynamic child, ie it is embedded in the container when created,
  // but not accessible via a member.
  // this will fail and go crazy if you try to aggregate an entity with dynamic members...
  // i should fix this...
  template <typename T>
      PropertyInstanceInformationTyped<PropType, T> *add(
          const PropertyInformationCreateData &data,
          const PropertyNameArg &name)
    {
    const PropertyInformation *newChildType = T::bootstrapStaticTypeInformation();

    PropertyInstanceInformation *inst = PropertyInformation::add(data, newChildType, name);

    return static_cast<PropertyInstanceInformationTyped<PropType, T> *>(inst);
    }

  template <typename T>
      PropertyInstanceInformationTyped<PropType, T> *add(
          const PropertyInformationCreateData &data,
          xsize location,
          const PropertyNameArg &name)
    {
    const PropertyInformation *newChildType = T::bootstrapStaticTypeInformation(data.allocator);

    PropertyInstanceInformation *inst =
      PropertyInformation::add(data, newChildType, location, name, false);

    return static_cast<PropertyInstanceInformationTyped<PropType, T> *>(inst);
    }

  template <typename PropTypeIn, typename InstanceTypeIn>
      PropertyInformationTyped<InstanceTypeIn> *
          extendContainedProperty(PropertyInstanceInformationTyped<PropTypeIn, InstanceTypeIn> *inst)
    {
    PropertyInformation *info = PropertyInformation::extendContainedProperty(inst);

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
  static void initiate(PropertyInformation *info, const char *typeName)
    {
    PropertyInformationFunctions fns;

    // update copy constructor too
    fns.createProperty = PropertyHelper<PropType>::create;
    fns.createPropertyInPlace = PropertyHelper<PropType>::createInPlace;
    fns.destroyProperty = PropertyHelper<PropType>::destroy;
    fns.createEmbeddedInstanceInformation = InstanceInformationHelper<PropType>::createEmbedded;
    fns.createDynamicInstanceInformation = InstanceInformationHelper<PropType>::createDynamic;
    fns.destroyEmbeddedInstanceInformation = InstanceInformationHelper<PropType>::destroyEmbedded;
    fns.destroyDynamicInstanceInformation = InstanceInformationHelper<PropType>::destroyDynamic;

    fns.save = PropType::saveProperty;
    fns.load = PropType::loadProperty;
    fns.shouldSave = PropType::shouldSaveProperty;
    fns.shouldSaveValue = PropType::shouldSavePropertyValue;
    fns.assign = PropType::assignProperty;

#ifdef S_PROPERTY_POST_CREATE
    fns.postCreate = 0;
#endif

    fns.createTypeInformation = (Functions::CreateTypeInformationFunction)PropType::createTypeInformation;

    info->setFunctions(fns);

    info->setVersion(PropType::Version);
    info->setSize(sizeof(PropType));
    info->setDynamicInstanceInformationSize(sizeof(typename PropType::DynamicInstanceInformation));
    info->setEmbeddedInstanceInformationSize(sizeof(typename PropType::EmbeddedInstanceInformation));

    PropType *offset = (PropType*)1;
    Property *propertyData = offset;
    xsize propertyDataOffset = (xsize)propertyData - 1;

    info->setPropertyDataOffset(propertyDataOffset);

    info->setExtendedParent(0);

    info->setTypeName(typeName);

    info->setInstances(0);

    ApiHelper<PropType>::create(info);
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
