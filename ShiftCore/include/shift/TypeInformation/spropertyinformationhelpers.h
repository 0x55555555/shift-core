#ifndef SPROPERTYINFORMATIONAPIUTILITIES_H
#define SPROPERTYINFORMATIONAPIUTILITIES_H

#include "XInterface.h"
#include "XTemporaryAllocator"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/spropertyinstanceinformation.h"
#include "shift/TypeInformation/spropertygroup.h"
#include "shift/TypeInformation/spropertytraits.h"
#include "shift/Properties/sproperty.h"
#include "shift/Properties/scontainer.h"
#include "shift/Properties/sattribute.inl"
#include "shift/Utilities/sresourcedescription.h"

namespace Shift
{

template <typename T> class PropertyInformationTyped;

#define S_IMPLEMENT_PROPERTY_BASE(myType, myIdentifier, grp)\
  static Shift::PropertyGroup::Information _##myIdentifier##StaticTypeInformation = \
    grp :: propertyGroup().registerPropertyInformation( \
    &_##myIdentifier##StaticTypeInformation, myType::bootstrapStaticTypeInformation); \
  const Shift::PropertyInformation *myType::staticTypeInformation() { return _##myIdentifier##StaticTypeInformation.information; }

#define S_IMPLEMENT_PROPERTY_EXPLICIT(INTRO, myType, myIdentifier, grp) S_IMPLEMENT_PROPERTY_BASE(myType, myIdentifier, grp) \
  INTRO const Shift::PropertyInformation *myType::bootstrapStaticTypeInformation(Eks::AllocatorBase *allocator) \
  { Shift::detail::checkType<myType>(); Shift::PropertyInformationTyped<myType>::bootstrapTypeInformation(&_##myIdentifier##StaticTypeInformation.information, \
  #myIdentifier, myType::ParentType::bootstrapStaticTypeInformation(allocator), allocator); return staticTypeInformation(); }

#define S_DEFAULT_INTRO
#define S_IMPLEMENT_PROPERTY(myType, grp) S_IMPLEMENT_PROPERTY_EXPLICIT(S_DEFAULT_INTRO, myType, myType, grp)

#define S_DEFAULT_TYPE_INFORMATION(name) \
void name::createTypeInformation(Shift::PropertyInformationTyped<name> *, \
    const Shift::PropertyInformationCreateData &) { } \

#define S_IMPLEMENT_PROPERTY_BASIC(myName, grp) S_IMPLEMENT_PROPERTY(myName, grp) \

#define S_IMPLEMENT_ABSTRACT_PROPERTY S_IMPLEMENT_PROPERTY

namespace detail
{
template <typename PropType> static void checkType()
  {
  typedef std::is_base_of<typename PropType::ParentType, PropType> Inherits;
  xCompileTimeAssert(Inherits::value == true);
  }

template <typename PropType, bool HasMetaType=QMetaTypeId2<PropType*>::Defined != 0> struct ApiHelper
  {

  };

template <typename PropType> struct ApiHelper<PropType, true>
  {
public:
  static void create(PropertyInformation *info)
    {
    const XScript::Interface<typename PropType::ParentType> *parentTempl =
        static_cast<XScript::Interface<typename PropType::ParentType>*>(PropType::ParentType::staticTypeInformation()->apiInterface());
    const XScript::Interface<Attribute> *baseTempl =
        static_cast<XScript::Interface<Attribute>*>(Attribute::staticTypeInformation()->apiInterface());

    XScript::Interface<PropType> *templ = XScript::Interface<PropType>::createWithParent(info->typeName(), parentTempl, baseTempl);
    info->setApiInterface(templ);
    }
  };

template <typename PropType> struct ApiHelper<PropType, false>
  {
public:
  static void create(PropertyInformation *info)
    {
    info->setApiInterface(PropType::ParentType::staticTypeInformation()->apiInterface());
    }
  };

template <> struct ApiHelper<Attribute, true>
  {
public:
  static void create(PropertyInformation *info)
    {
    XScript::Interface<Attribute> *templ = XScript::Interface<Attribute>::create(info->typeName().data());
    info->setApiInterface(templ);
    }
  };

template <typename T, void FUNC( const PropertyInstanceInformation *, T * )> struct ComputeHelper
  {
  static void compute(const PropertyInstanceInformation *c, Property *prop)
    {
    T* t = prop->uncheckedCastTo<T>();
    xAssert(t);
    FUNC(c, t);
    }
  };

template <typename T, void FUNC( T * )> struct ComputeNoInstanceInformationHelper
  {
  static void compute(const PropertyInstanceInformation *, Property *prop)
    {
    T* t = prop->uncheckedCastTo<T>();
    xAssert(t);
    FUNC(t);
    }
  };

template <typename PropType, typename Fn> struct CompteLambdaHelper
  {
  static void bind(const PropertyInstanceInformation *, Container* par)
    {
    Fn method;

    method(par->uncheckedCastTo<PropType>());
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
    Function t = (Function)detail::ComputeNoInstanceInformationHelper<PropType, FUNC>::compute;

    setCompute(t);
    }

  template <typename Fn>
      void setCompute(Fn)
    {
    setCompute(&detail::CompteLambdaHelper<PropType, Fn>::bind);
    }

  template <void FUNC( const PropertyInstanceInformation *, PropType * )>
      void setComputeWithInstanceInformation()
    {
    Function t = (Function)detail::ComputeHelper<PropType, FUNC>::compute;

    setCompute(t);
    }
  };

class SHIFT_EXPORT PropertyInformationChildrenCreator
  {
public:
  EmbeddedPropertyInstanceInformation *add(
      const PropertyInformation *newChildType,
      xsize location,
      const NameArg &,
      bool notClassMember);

  EmbeddedPropertyInstanceInformation *add(
      const PropertyInformation *newChildType,
      const NameArg &);

  xsize *createAffects(
      const EmbeddedPropertyInstanceInformation **info,
      xsize size);

  static xsize *createAffects(
      Eks::AllocatorBase *allocator,
      const EmbeddedPropertyInstanceInformation **info,
      xsize size);

  const EmbeddedPropertyInstanceInformation *child(xsize location) const;
  EmbeddedPropertyInstanceInformation *overrideChild(xsize location);

  PropertyInformationChildrenCreator(
          PropertyInformationChildrenCreator&& data) :
  _temporaryAllocator(TypeRegistry::temporaryAllocator()),
    _properties(&_temporaryAllocator), _data(data._data)
  {
      _properties = data._properties;

  }


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
          const NameArg &name)
    {
    // this isnt always true, but normally is.
    // when adding extended child properties things get weird.
    //
    //typedef std::is_base_of<AncestorPropType, PropType> Inherits;
    //xCompileTimeAssert(Inherits::value == true);

    // If a class has this flag set it has children after its allocation which are not specified
    // in the class definition. This type of class can only be added to PropertyInformation
    // without a location
    //
    xCompileTimeAssert(U::HasDynamicChildren == false);

    xptrdiff location = findLocation(ptr);

    xsize offset = 0;
    _information->findAllocatableBase(offset);
    location -= offset;

    return add<U>(location, name);
    }


  template <typename U, typename AncestorPropType, xsize SIZE>
      void addArray(
          U (AncestorPropType::* ptr)[SIZE],
          const NameArg &name,
          PropertyInstanceInformationTyped<PropType, U> **inst=0)
    {
    PropertyInstanceInformationTyped<PropType, U> *instArray[SIZE];
    inst = inst ? inst : instArray;

    // If a class has this flag set it has children after its allocation which are not specified
    // in the class definition. This type of class can only be added to PropertyInformation
    // without a location
    //
    xCompileTimeAssert(U::HasDynamicChildren == false);

    Name str;
    name.toName(str);
    const xsize nameLength = str.length();

    AncestorPropType *prop = (AncestorPropType *)0x0;

    for(xsize i = 0; i < SIZE; ++i)
      {
      str.appendType(i);

      union
        {
        U *in;
        U AncestorPropType::*out;
        } conv;

      conv.in = &((prop->*ptr)[i]);

      U AncestorPropType::* elem = conv.out;

      inst[i] = add(elem, str);

      str.resize(nameLength, '\0');
      }
    }

  // add a dynamic child, ie it is embedded in the container when created,
  // but not accessible via a member.
  // this will fail and go crazy if you try to aggregate an entity with dynamic members...
  // i should fix this...
  template <typename T>
      PropertyInstanceInformationTyped<PropType, T> *add(
          const NameArg &name)
    {
    const PropertyInformation *newChildType = T::bootstrapStaticTypeInformation(_data.allocator);

    // User must set this to indicate to users of their container that they may not be
    // added statically to classes.
    //
    xCompileTimeAssert(PropType::HasDynamicChildren != false);

    EmbeddedPropertyInstanceInformation *inst =
        PropertyInformationChildrenCreator::add(newChildType, name);

    return static_cast<PropertyInstanceInformationTyped<PropType, T> *>(inst);
    }

  template <typename T>
      PropertyInstanceInformationTyped<PropType, T> *add(
          xsize location,
          const NameArg &name)
    {
    const PropertyInformation *newChildType = T::bootstrapStaticTypeInformation(_data.allocator);

    // If a class has this flag set it has children after its allocation which are not specified
    // in the class definition. This type of class can only be added to PropertyInformation
    // without a location
    //
    xCompileTimeAssert(T::HasDynamicChildren == false);

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
    Container *container = static_cast<Container *>(u);
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

    info->setFormat(Eks::ResourceDescriptionTypeHelper<PropType>::createFor());
    info->setDynamicInstanceInformationFormat(
      Eks::ResourceDescriptionTypeHelper<typename PropType::DynamicInstanceInformation>::createFor());
    info->setEmbeddedInstanceInformationFormat(
      Eks::ResourceDescriptionTypeHelper<typename PropType::EmbeddedInstanceInformation>::createFor());

    PropType *offset = (PropType*)1;
    Attribute *propertyData = offset;
    xsize propertyDataOffset = (xsize)propertyData - 1;

    info->setPropertyDataOffset(propertyDataOffset);

    info->setExtendedParent(0);

    info->setTypeName(typeName);

    info->setInstances(0);

    detail::ApiHelper<PropType>::create(info);

    PropertyInformationCreateData data(allocator);
    data.registerAttributes = true;
    data.registerInterfaces = true;
    PropType::createTypeInformation(info, data);
    }
  };

template <typename R, typename T> class PropertyAffectsWalker
  {
public:
  PropertyAffectsWalker(T *c, const xsize *aff) : _affects(aff), _container(c)
    {
    }

  class Iterator
    {
  public:
    Iterator(const xsize *s, T *c) : _affects(s), _container(c)
      {
      }

    void operator++()
      {
      ++_affects;
      }

    R *operator*() const
      {
      xuint8* parentLocation = (xuint8*)_container;

      const xuint8* affectedLocation = parentLocation + *_affects;
      return (R*)affectedLocation;
      }

    bool operator!=(const Iterator &i) const
      {
      xsize a = _affects ? *_affects : 0;
      xsize b = i._affects ? *i._affects : 0;
      return a != b;
      }

  private:
    const xsize *_affects;
    T *_container;
    };

  Iterator begin() const { return Iterator(_affects, _container); }
  Iterator end() const { return Iterator(0, _container); }

private:
  const xsize *_affects;
  T *_container;
  };

inline PropertyAffectsWalker<Property, Container> EmbeddedPropertyInstanceInformation::affectsWalker(Container *c) const
  {
  return PropertyAffectsWalker<Property, Container>(c, _affects);
  }

inline PropertyAffectsWalker<const Property, const Container> EmbeddedPropertyInstanceInformation::affectsWalker(const Container *c) const
  {
  return PropertyAffectsWalker<const Property, const Container>(c, _affects);
  }
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
