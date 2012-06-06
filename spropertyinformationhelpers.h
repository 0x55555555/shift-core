#ifndef SPROPERTYINFORMATIONAPIUTILITIES_H
#define SPROPERTYINFORMATIONAPIUTILITIES_H

#include "spropertyinformation.h"
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
    const XInterface<PropType::ParentType> *parentTempl =
        static_cast<XInterface<PropType::ParentType>*>(PropType::ParentType::staticTypeInformation()->apiInterface());
    const XInterface<SProperty> *baseTempl =
        static_cast<XInterface<SProperty>*>(SProperty::staticTypeInformation()->apiInterface());

    XInterface<PropType> *templ = XInterface<PropType>::createWithParent(info->typeName(), parentTempl, baseTempl);
    info->setApiInterface(templ);
    }
  };

template <> struct ApiHelper<SProperty>
  {
public:
  static void create(SPropertyInformation *info)
    {
    XInterface<SProperty> *templ = XInterface<SProperty>::create(info->typeName());
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
  static SPropertyInstanceInformation *create(void *allocation)
    {
    return new(allocation) T::InstanceInformation;
    }
  static void destroy(SPropertyInstanceInformation *allocation)
    {
    ((T*)allocation)->~T();
    }
  };

template <typename T, void FUNC( const SPropertyInstanceInformation *, T * )> struct ComputeHelper
  {
  static void compute( const SPropertyInstanceInformation *c, SProperty *prop)
    {
    T* t = prop->castTo<T>();
    xAssert(t);
    FUNC(c, t);
    }
  };
}

template <typename PropType, typename InstanceType> class SPropertyInstanceInformationTyped : public InstanceType
  {
public:
  using InstanceType::setCompute;

  template <void FUNC( const SPropertyInstanceInformation *, PropType * )>
      void setCompute()
    {
    ComputeFunction t = (ComputeFunction)ComputeHelper<PropType, FUNC>::compute;

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

  using SPropertyInformation::child;

  template <typename U>
  SPropertyInstanceInformationTyped<PropType, typename U::InstanceInformation> *child(U PropType::* ptr)
    {
    xsize location = findLocation(ptr);

    return static_cast<SPropertyInstanceInformationTyped<PropType,
                                                         typename U::InstanceInformation>*>(child(location));
  }

  template <typename U>
  const SPropertyInstanceInformationTyped<PropType, typename U::InstanceInformation> *child(U PropType::* ptr) const
    {
    xsize location = findLocation(ptr);

    return static_cast<const SPropertyInstanceInformationTyped<PropType,
                                                               typename U::InstanceInformation>*>(child(location));
    }

  XInterface<PropType> *apiInterface()
    {
    return static_cast<XInterface<PropType>*>(SPropertyInformation::apiInterface());
    }

  const XInterface<PropType> *apiInterface() const
    {
    return static_cast<const XInterface<PropType>*>(SPropertyInformation::apiInterface());
    }

  static SPropertyInformation *createTypeInformation(const char *name, const SPropertyInformation *parentType)
    {
    SProfileFunction

    typedef void (*FnType)(SPropertyInformation *, const char *);
    FnType fn = SPropertyInformationTyped<PropType>::initiate;

    return createTypeInformationInternal(name, parentType, fn);
    }

  template <typename U> xsize findLocation(U PropType::* ptr)
    {
    PropType *u = reinterpret_cast<PropType*>(1); // avoid special casing for zero static cast
    SPropertyContainer *container = static_cast<SPropertyContainer *>(u);
    U *offset = &(u->*ptr);

    SProperty *propOffset = offset;

    // one added earlier is cancelled out because the 1 is counted in both offset and container
    xptrdiff location = reinterpret_cast<xsize>(propOffset) - reinterpret_cast<xsize>(container);
    xAssert(location > 0);

    return (xsize)location;
    }

  template <typename U>
      SPropertyInstanceInformationTyped<PropType, typename U::InstanceInformation> *add(U PropType::* ptr,
                                                                                    const QString &name)
    {
    xptrdiff location = findLocation(ptr);

    if(extendedParent())
      {
      location -= extendedParent()->location();
      }

    return add<U>(location, name);
    }

  template <typename T>
      SPropertyInstanceInformationTyped<PropType, typename T::InstanceInformation> *add(const QString &name)
    {
    const SPropertyInformation *newChildType = T::bootstrapStaticTypeInformation();

    SPropertyInstanceInformation *inst = add(newChildType, name);

    return static_cast<SPropertyInstanceInformationTyped<PropType, typename T::InstanceInformation> *>(inst);
    }

  template <typename T>
      SPropertyInstanceInformationTyped<PropType, typename T::InstanceInformation> *add(xsize location,
                                                                                        const QString &name)
    {
    const SPropertyInformation *newChildType = T::bootstrapStaticTypeInformation();

    SPropertyInstanceInformation *inst = SPropertyInformation::add(newChildType, location, name, false);

    return static_cast<SPropertyInstanceInformationTyped<PropType, typename T::InstanceInformation> *>(inst);
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

    addInterfaceFactory(new InheritedInterface);
    }

  template <typename PropType, typename InstanceType>
      SPropertyInformationTyped<PropType> *
          extendContainedProperty(SPropertyInstanceInformationTyped<PropType, InstanceType> *inst)
    {
    SPropertyInformation *info = SPropertyInformation::extendContainedProperty(inst);

    return static_cast<SPropertyInformationTyped<PropType>*>(info);
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
    fns.createInstanceInformation = InstanceInformationHelper<PropType>::create;
    fns.destroyInstanceInformation = InstanceInformationHelper<PropType>::destroy;

    fns.save = PropType::saveProperty;
    fns.load = PropType::loadProperty;
    fns.shouldSave = PropType::shouldSaveProperty;
    fns.shouldSaveValue = PropType::shouldSavePropertyValue;
    fns.assign = PropType::assignProperty;
    fns.postChildSet = PropType::postChildSet;

#ifdef S_PROPERTY_POST_CREATE
    fns.postCreate = 0;
#endif

    fns.createTypeInformation = (Functions::CreateTypeInformationFunction)PropType::createTypeInformation;

    info->setFunctions(fns);

    info->setVersion(PropType::Version);
    info->setSize(sizeof(PropType));
    info->setInstanceInformationSize(sizeof(typename PropType::InstanceInformation));

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

namespace XScriptConvert
{
namespace internal
{
template <> struct SHIFT_EXPORT JSToNative<SPropertyInformation>
  {
  typedef const SPropertyInformation *ResultType;

  ResultType operator()(XScriptValue const &h) const;
  };

template <> struct SHIFT_EXPORT NativeToJS<SPropertyInformation>
  {
  XScriptValue operator()(const SPropertyInformation *x) const;
  XScriptValue operator()(const SPropertyInformation &x) const;
  };
}
}

#endif // SPROPERTYINFORMATIONAPIUTILITIES_H
