#ifndef SPROPERTYINFORMATIONAPIUTILITIES_H
#define SPROPERTYINFORMATIONAPIUTILITIES_H

#include "spropertyinformation.h"
#include "spropertyinstanceinformation.h"
#include "sproperty.inl"
#include "spropertygroup.h"
#include "XInterface.h"
#include "sproperty.h"

#define S_IMPLEMENT_PROPERTY(myName, grp) \
  static SPropertyGroup::Information _##myName##StaticTypeInformation = \
    grp :: propertyGroup().registerPropertyInformation( \
    &_##myName##StaticTypeInformation, myName::bootstrapStaticTypeInformation); \
  const SPropertyInformation *myName::staticTypeInformation() { return _##myName##StaticTypeInformation.information; } \
  const SPropertyInformation *myName::bootstrapStaticTypeInformation() \
  { SPropertyInformationTyped<myName>::bootstrapTypeInformation(&_##myName##StaticTypeInformation.information, \
  #myName, myName::ParentType::bootstrapStaticTypeInformation()); return staticTypeInformation(); }

#define S_IMPLEMENT_ABSTRACT_PROPERTY(myName, grp) \
  S_IMPLEMENT_PROPERTY(myName, grp)

namespace
{
template <typename PropType> struct ApiHelper
  {
public:
  static void create(SPropertyInformation *info)
    {
    const XScript::Interface<typename PropType::ParentType> *parentTempl =
        static_cast<XScript::Interface<typename PropType::ParentType>*>(PropType::ParentType::staticTypeInformation()->apiInterface());
    const XScript::Interface<SProperty> *baseTempl =
        static_cast<XScript::Interface<SProperty>*>(SProperty::staticTypeInformation()->apiInterface());

    XScript::Interface<PropType> *templ = XScript::Interface<PropType>::createWithParent(info->typeName(), parentTempl, baseTempl);
    info->setApiInterface(templ);
    }
  };

template <> struct ApiHelper<SProperty>
  {
public:
  static void create(SPropertyInformation *info)
    {
    XScript::Interface<SProperty> *templ = XScript::Interface<SProperty>::create(info->typeName());
    info->setApiInterface(templ);
    }
  };

template<typename T, bool Abstract = T::IsAbstract> struct PropertyHelper;

template<typename T> struct PropertyHelper<T, true>
  {
  static SProperty *create(void *)
    {
    xAssertFail();
    return 0;
    }
  static void createInPlace(SProperty *)
    {
    xAssertFail();
    }
  static void destroy(SProperty *)
    {
    xAssertFail();
    }
  };

template<typename T> struct PropertyHelper<T, false>
  {
  static SProperty *create(void *ptr)
    {
    return new(ptr) T();
    }
  static void createInPlace(SProperty *ptr)
    {
    T* t = static_cast<T*>(ptr);
    new(t) T();
    }
  static void destroy(SProperty *ptr)
    {
    ((T*)ptr)->~T();
    }
  };

template <typename T> struct InstanceInformationHelper
  {
  typedef typename T::DynamicInstanceInformation DyInst;
  typedef typename T::StaticInstanceInformation StInst;
  static SPropertyInstanceInformation *createDynamic(void *allocation)
    {
    return new(allocation) typename DyInst;
    }
  static SPropertyInstanceInformation *createStatic(void *allocation)
    {
    return new(allocation) typename StInst;
    }
  static void destroyDynamic(SPropertyInstanceInformation *allocation)
    {
    ((DyInst*)allocation)->~DyInst();
    }
  static void destroyStatic(SPropertyInstanceInformation *allocation)
    {
    ((StInst*)allocation)->~StInst();
    }
  };

template <typename T, void FUNC( const SPropertyInstanceInformation *, T * )> struct ComputeHelper
  {
  static void compute( const SPropertyInstanceInformation *c, SProperty *prop)
    {
    T* t = prop->uncheckedCastTo<T>();
    xAssert(t);
    FUNC(c, t);
    }
  };

template <typename T, void FUNC( T * )> struct ComputeNoInstanceInformationHelper
  {
  static void compute( const SPropertyInstanceInformation *, SProperty *prop)
    {
    T* t = prop->uncheckedCastTo<T>();
    xAssert(t);
    FUNC(t);
    }
  };
}

template <typename PropType, typename InstanceType> class SPropertyInstanceInformationTyped : public InstanceType::StaticInstanceInformation
  {
public:
  using InstanceType::StaticInstanceInformation::setCompute;
  typedef typename InstanceType::StaticInstanceInformation::ComputeFunction Function;

  template <void FUNC(PropType * )>
      void setCompute()
    {
    Function t = (Function)ComputeNoInstanceInformationHelper<PropType, FUNC>::compute;

    setCompute(t);
    }

  template <void FUNC( const SPropertyInstanceInformation *, PropType * )>
      void setComputeWithInstanceInformation()
    {
    Function t = (Function)ComputeHelper<PropType, FUNC>::compute;

    setCompute(t);
    }
  };

template <typename PropType> class SPropertyInformationTyped : public SPropertyInformation
  {
public:
  static void bootstrapTypeInformation(SPropertyInformation **info,
                               const char *name,
                               const SPropertyInformation *parent)
    {
    if(!*info)
      {
      *info = createTypeInformation(name, parent);
      }
    }

  template <typename U, typename PropTypeAncestor>
  SPropertyInstanceInformationTyped<PropType, U> *child(U PropTypeAncestor::* ptr)
    {
    xsize location = findLocation(ptr);

    return static_cast<SPropertyInstanceInformationTyped<PropType, U>*>(SPropertyInformation::child(location));
    }

  template <typename U, typename PropTypeAncestor>
  const SPropertyInstanceInformationTyped<PropType, U> *child(U PropTypeAncestor::* ptr) const
    {
    xsize location = findLocation(ptr);

    xsize offset = 0;
    SPropertyInformation* allocatable = findAllocatableBase(offset);
    location -= offset;

    return static_cast<const SPropertyInstanceInformationTyped<PropType, U>*>(SPropertyInformation::child(location));
    }

  XScript::Interface<PropType> *apiInterface()
    {
    return static_cast<XScript::Interface<PropType>*>(SPropertyInformation::apiInterface());
    }

  const XScript::Interface<PropType> *apiInterface() const
    {
    return static_cast<const XScript::Interface<PropType>*>(SPropertyInformation::apiInterface());
    }

  static SPropertyInformation *createTypeInformation(const char *name, const SPropertyInformation *parentType)
    {
    typedef void (*FnType)(SPropertyInformation *, const char *);
    FnType fn = SPropertyInformationTyped<PropType>::initiate;

    return createTypeInformationInternal(name, parentType, fn);
    }

  template <typename U, typename AncestorPropType> xsize findLocation(U AncestorPropType::* ptr)
    {
    AncestorPropType *u = reinterpret_cast<AncestorPropType*>(1); // avoid special casing for zero static cast
    SPropertyContainer *container = static_cast<SPropertyContainer *>(u);
    U *offset = &(u->*ptr);

    SProperty *propOffset = offset;

    // one added earlier is cancelled out because the 1 is counted in both offset and container
    xptrdiff location = reinterpret_cast<xsize>(propOffset) - reinterpret_cast<xsize>(container);
    xAssert(location > 0);

    return (xsize)location;
    }

  template <typename U, typename AncestorPropType>
      SPropertyInstanceInformationTyped<PropType, U> *add(U AncestorPropType::* ptr, const QString &name)
    {
    xptrdiff location = findLocation(ptr);

    xsize offset = 0;
    findAllocatableBase(offset);
    location -= offset;

    return add<U>(location, name);
    }

  // add a dynamic child, ie it is embedded in the container when created,
  // but not accessible via a member.
  // this will fail and go crazy if you try to aggregate an entity with dynamic members...
  // i should fix this...
  template <typename T>
      SPropertyInstanceInformationTyped<PropType, T> *add(const QString &name)
    {
    const SPropertyInformation *newChildType = T::bootstrapStaticTypeInformation();

    SPropertyInstanceInformation *inst = SPropertyInformation::add(newChildType, name);

    return static_cast<SPropertyInstanceInformationTyped<PropType, T> *>(inst);
    }

  template <typename T>
      SPropertyInstanceInformationTyped<PropType, T> *add(xsize location, const QString &name)
    {
    const SPropertyInformation *newChildType = T::bootstrapStaticTypeInformation();

    SPropertyInstanceInformation *inst = SPropertyInformation::add(newChildType, location, name, false);

    return static_cast<SPropertyInstanceInformationTyped<PropType, T> *>(inst);
    }

  template <typename T> void addInheritedInterface()
    {
    class InheritedInterface : public SInterfaceBaseFactory
      {
      S_INTERFACE_FACTORY_TYPE(T)
    public:
      InheritedInterface() : SInterfaceBaseFactory(true) { }
      virtual SInterfaceBase *classInterface(SProperty *prop)
        {
        return prop->castTo<PropType>();
        }
      };

    SPropertyInformation::addInterfaceFactoryInternal(InheritedInterface::InterfaceType::InterfaceTypeId, new InheritedInterface);
    }

  template <typename PropTypeIn, typename InstanceTypeIn>
      SPropertyInformationTyped<InstanceTypeIn> *
          extendContainedProperty(SPropertyInstanceInformationTyped<PropTypeIn, InstanceTypeIn> *inst)
    {
    SPropertyInformation *info = SPropertyInformation::extendContainedProperty(inst);

    return static_cast<SPropertyInformationTyped<InstanceTypeIn>*>(info);
    }

#ifdef S_PROPERTY_USER_DATA
  template <typename T> void addAddonInterface() const
    {
    class AddonInterface : public SInterfaceBaseFactory
      {
      S_INTERFACE_FACTORY_TYPE(T)
      AddonInterface() : SInterfaceBaseFactory(true) { }
      virtual SInterfaceBase *classInterface(SProperty *prop)
        {
        SProperty::UserData *userData = prop->firstUserData();
        while(userData)
          {
          if(userData->userDataTypeId() == SUserDataTypes::InterfaceUserDataType)
            {
            SInterfaceBase *interfaceBase = static_cast<SInterfaceBase*>(userData);
            if(interfaceBase->interfaceTypeId() == T::InterfaceType::InterfaceTypeId)
              {
              return interfaceBase;
              }
            }
          userData = userData->next();
          }

        // none found, create one and add it.
        T* newInterface = new T(prop);
        prop->addUserData(newInterface);
        return newInterface;
        }
      };

    addInterfaceFactory(new AddonInterface);
    }
#endif

private:
  static void initiate(SPropertyInformation *info, const char *typeName)
    {
    SPropertyInformationFunctions fns;

    // update copy constructor too
    fns.createProperty = PropertyHelper<PropType>::create;
    fns.createPropertyInPlace = PropertyHelper<PropType>::createInPlace;
    fns.destroyProperty = PropertyHelper<PropType>::destroy;
    fns.createStaticInstanceInformation = InstanceInformationHelper<PropType>::createStatic;
    fns.createDynamicInstanceInformation = InstanceInformationHelper<PropType>::createDynamic;
    fns.destroyStaticInstanceInformation = InstanceInformationHelper<PropType>::destroyStatic;
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
    info->setStaticInstanceInformationSize(sizeof(typename PropType::StaticInstanceInformation));

    PropType *offset = (PropType*)1;
    SProperty *propertyData = offset;
    xsize propertyDataOffset = (xsize)propertyData - 1;

    info->setPropertyDataOffset(propertyDataOffset);

    info->setExtendedParent(0);

    info->setTypeName(typeName);

    info->setInstances(0);

    ApiHelper<PropType>::create(info);
    }
  };

namespace XScript
{
namespace Convert
{
namespace internal
{
template <> struct SHIFT_EXPORT JSToNative<SPropertyInformation>
  {
  typedef const SPropertyInformation *ResultType;

  ResultType operator()(Value const &h) const;
  };

template <> struct SHIFT_EXPORT NativeToJS<SPropertyInformation>
  {
  Value operator()(const SPropertyInformation *x) const;
  Value operator()(const SPropertyInformation &x) const;
  };
}
}
}

#endif // SPROPERTYINFORMATIONAPIUTILITIES_H
