#ifndef SPROPERTYTRAITS_H
#define SPROPERTYTRAITS_H

#include "spropertyinformation.h"

namespace Shift
{

template <typename TypeTraitsCreation, bool Abstract> class PropertyCreateSelector
  {
public:
  static Property *createProperty(void *ptr)
    {
    return TypeTraitsCreation::createProperty(ptr);
    }
  static void createPropertyInPlace(Property *ptr)
    {
    TypeTraitsCreation::createPropertyInPlace(ptr);
    }
  static void destroyProperty(Property *ptr)
    {
    TypeTraitsCreation::destroyProperty(ptr);
    }
  };

template <typename TypeTraits> class PropertyCreateSelector<TypeTraits, true>
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
    typedef typename PropType::Traits Traits;
    typedef typename Traits::template TypeTraits<PropType>::Type TypeTraits;

    typedef typename TypeTraits::template Creation<PropType> TypeTraitsCreation;
    typedef PropertyCreateSelector<TypeTraitsCreation, PropType::IsAbstract> CreateSelection;

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
    fns.assign =
        (PropertyInformationFunctions::AssignFunction)
          Traits::assignProperty;

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

  template <typename Type> struct Creation
    {
    static Property *createProperty(void *ptr)
      {
      return new(ptr) Type();
      }
    static void createPropertyInPlace(Property *ptr)
      {
      Type* t = static_cast<Type*>(ptr);
      new(t) Type();
      }
    static void destroyProperty(Property *ptr)
      {
      (void)ptr;
      ((Type*)ptr)->~Type();
      }
    };

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

template <typename T> class BasePODPropertyTraits : public PropertyBaseTraits
  {
public:
  static void saveProperty(const Property *p, Saver &l )
    {
    PropertyBaseTraits::saveProperty(p, l);
    }

  static Property *loadProperty(PropertyContainer *parent, Loader &l)
    {
    Property *prop = PropertyBaseTraits::loadProperty(parent, l);
    return prop;
    }

  static bool shouldSavePropertyValue(const Property *)
    {
    return false;
    }

  enum
    {
    assignProperty = 0
    };
  };

template <typename T> class PODPropertyTraits : public BasePODPropertyTraits<T>
  {
public:
  static void saveProperty(const Property *p, Saver &l )
    {
    BasePODPropertyTraits<T>::saveProperty(p, l);
    const T *ptr = p->uncheckedCastTo<T>();
    writeValue(l, ptr->_value);
    }

  static Property *loadProperty(PropertyContainer *parent, Loader &l)
    {
    Property *prop = BasePODPropertyTraits<T>::loadProperty(parent, l);
    T *ptr = prop->uncheckedCastTo<T>();
    readValue(l, ptr->_value);
    return prop;
    }

  static bool shouldSavePropertyValue(const Property *p)
    {
    const T *ptr = p->uncheckedCastTo<T>();

    if(BasePODPropertyTraits<T>::shouldSavePropertyValue(p))
      {
      using ::operator!=;

      if(ptr->isDynamic() ||
         ptr->value() != ptr->embeddedInstanceInformation()->defaultValue())
        {
        return true;
        }
      }

    return false;
    }

  static void assignProperty(const Shift::Property *p, Shift::Property *l )
    {
    T::assignProperty(p, l);
    }
  };

}
}

#endif // SPROPERTYTRAITS_H
