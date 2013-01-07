#ifndef SPROPERTYTRAITS_H
#define SPROPERTYTRAITS_H

#include "spropertyinformation.h"

namespace Shift
{

template <typename TypeTraits, bool Abstract> class PropertyCreateSelector
  {
public:
  static Property *createProperty(void *ptr)
    {
    return TypeTraits::createProperty(ptr);
    }
  static void createPropertyInPlace(Property *ptr)
    {
    TypeTraits::createPropertyInPlace(ptr);
    }
  static void destroyProperty(Property *ptr)
    {
    TypeTraits::destroyProperty(ptr);
    }
  };

template <typename TypeTraits> class PropertyCreateSelector<TypeTraits, false>
  {
public:
  enum
    {
    createProperty = 0,
    createPropertyInPlace = 0,
    destroyProperty = 0
    };
  };


class PropertyTraits
  {
public:
  template <typename PropType> static void build(PropertyInformationFunctions &fns)
    {
    typedef PropType::Traits Traits;
    typedef Traits::TypeTraits<PropType>::Type TypeTraits;

    typedef PropertyCreateSelector<TypeTraits, PropType::IsAbstract> CreateSelection;

    fns.createProperty =
        (PropertyInformationFunctions::CreatePropertyFunction)
          CreateSelection::createProperty;
    fns.destroyProperty =
        (PropertyInformationFunctions::DestroyPropertyFunction)
          CreateSelection::destroyProperty;
    fns.createPropertyInPlace =
        (PropertyInformationFunctions::CreatePropertyInPlaceFunction)
          CreateSelection::createPropertyInPlace;

    fns.createEmbeddedInstanceInformation = TypeTraits::createEmbeddedInstanceInformation;
    fns.createDynamicInstanceInformation = TypeTraits::createDynamicInstanceInformation;
    fns.destroyEmbeddedInstanceInformation = TypeTraits::destroyEmbeddedInstanceInformation;
    fns.destroyDynamicInstanceInformation = TypeTraits::destroyDynamicInstanceInformation;

    fns.createTypeInformation =
        (PropertyInformationFunctions::CreateTypeInformationFunction)
          PropType::createTypeInformation;

    fns.save = Traits::saveProperty;
    fns.load = Traits::loadProperty;
    fns.shouldSave = Traits::shouldSaveProperty;
    fns.shouldSaveValue = Traits::shouldSavePropertyValue;
    fns.assign = Traits::assignProperty;

  #ifdef S_PROPERTY_POST_CREATE
    fns.postCreate = 0;
  #endif

    }
  };

namespace detail
{

template <typename T> class PropertyBaseTypeTraits
  {
public:
  typedef typename T::DynamicInstanceInformation DyInst;
  typedef typename T::EmbeddedInstanceInformation StInst;

  static Property *createProperty(void *ptr)
    {
    return new(ptr) T();
    }
  static void createPropertyInPlace(Property *ptr)
    {
    T* t = static_cast<T*>(ptr);
    new(t) T();
    }
  static void destroyProperty(Property *ptr)
    {
    (void)ptr;
    ((T*)ptr)->~T();
    }
  static PropertyInstanceInformation *createDynamicInstanceInformation(void *allocation)
    {
    return new(allocation) DyInst;
    }
  static PropertyInstanceInformation *createEmbeddedInstanceInformation(void *allocation)
    {
    return new(allocation) StInst;
    }
  static void destroyDynamicInstanceInformation(PropertyInstanceInformation *allocation)
    {
    ((DyInst*)allocation)->~DyInst();
    }
  static void destroyEmbeddedInstanceInformation(PropertyInstanceInformation *allocation)
    {
    ((StInst*)allocation)->~StInst();
    }
  };

class SHIFT_EXPORT PropertyBaseTraits
  {
public:
  static void assignProperty(const Property *, Property *);
  static void saveProperty(const Property *, Saver & );
  static Property *loadProperty(PropertyContainer *, Loader &);

  // should this properties value be saved, for example not when the value
  // is this property's value the default as it is when created.
  static bool shouldSavePropertyValue(const Property *);

  // should the property definition itself be saved, note this function must be true if the above is true
  // but the above can be false when this is true.
  static bool shouldSaveProperty(const Property *);

  // helper for custom saving, allows not saving input specifically.
  static void saveProperty(const Property *, Saver &, bool writeInput);

  // traits customised for each derived type
  template <typename T> struct TypeTraits
    {
    typedef PropertyBaseTypeTraits<T> Type;
    };
  };

class SHIFT_EXPORT PropertyContainerTraits : public PropertyBaseTraits
  {
public:
  static void assignProperty(const Property *, Property *);
  static void saveProperty(const Property *, Saver & );
  static Property *loadProperty(PropertyContainer *, Loader &);
  static bool shouldSavePropertyValue(const Property *);
  };

}
}

#endif // SPROPERTYTRAITS_H
