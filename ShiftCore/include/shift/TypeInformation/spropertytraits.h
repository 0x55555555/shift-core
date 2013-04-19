#ifndef SPROPERTYTRAITS_H
#define SPROPERTYTRAITS_H

#include "spropertyinformation.h"

namespace Shift
{

template <typename TypeTraitsCreation, bool Abstract> class PropertyCreateSelector
  {
public:
  static Attribute *create(void *ptr)
    {
    return TypeTraitsCreation::create(ptr);
    }
  static void createInPlace(Attribute *ptr)
    {
    TypeTraitsCreation::createInPlace(ptr);
    }
  static void *destroy(Attribute *ptr)
    {
    return TypeTraitsCreation::destroy(ptr);
    }
  };

template <typename TypeTraits> class PropertyCreateSelector<TypeTraits, true>
  {
public:
  enum
    {
    create = 0,
    createInPlace = 0,
    destroy = 0
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

    fns.create =
        (PropertyInformationFunctions::CreateFunction)
          CreateSelection::create;
    fns.destroy =
        (PropertyInformationFunctions::DestroyFunction)
          CreateSelection::destroy;
    fns.createInPlace =
        (PropertyInformationFunctions::CreateInPlaceFunction)
          CreateSelection::createInPlace;

    fns.createEmbeddedInstanceInformation = TypeTraits::createEmbeddedInstanceInformation;
    fns.createDynamicInstanceInformation = TypeTraits::createDynamicInstanceInformation;
    fns.destroyEmbeddedInstanceInformation = TypeTraits::destroyEmbeddedInstanceInformation;
    fns.destroyDynamicInstanceInformation = TypeTraits::destroyDynamicInstanceInformation;

    fns.save = Traits::save;
    fns.load = Traits::load;
    fns.shouldSave = Traits::shouldSave;
    fns.shouldSaveValue = Traits::shouldSaveValue;
    fns.assign =
        (PropertyInformationFunctions::AssignFunction)
          Traits::assign;

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
    static Attribute *create(void *ptr)
      {
      Type *t = new(ptr) Type();
      xAssert(t == ptr);

      return t;
      }
    static void createInPlace(Attribute *ptr)
      {
      Type* t = static_cast<Type*>(ptr);
      new(t) Type();
      }
    static void *destroy(Attribute *ptr)
      {
      (void)ptr;
      ((Type*)ptr)->~Type();

      return (Type*)ptr;
      }
    };

  static PropertyInstanceInformation *createDynamicInstanceInformation(
      void *allocation,
      const PropertyInstanceInformation *cpy)
    {
    if(cpy)
      {
      return new(allocation) DyInst(*static_cast<const DyInst*>(cpy));
      }

    return new(allocation) DyInst;
    }
  static PropertyInstanceInformation *createEmbeddedInstanceInformation(
      void *allocation,
      const PropertyInstanceInformation *cpy)
    {
    if(cpy)
      {
      return new(allocation) StInst(*static_cast<const StInst*>(cpy));
      }

    return new(allocation) StInst;
    }
  static void *destroyDynamicInstanceInformation(PropertyInstanceInformation *allocation)
    {
    ((DyInst*)allocation)->~DyInst();
    return ((DyInst*)allocation);
    }
  static void *destroyEmbeddedInstanceInformation(PropertyInstanceInformation *allocation)
    {
    ((StInst*)allocation)->~StInst();
    return ((StInst*)allocation);
    }
  };

class SHIFT_EXPORT PropertyBaseTraits
  {
public:
  static void assign(const Attribute *, Attribute *);
  static void save(const Attribute *, Saver & );
  static Attribute *load(Container *, Loader &);

  // should this properties value be saved, for example not when the value
  // is this property's value the default as it is when created.
  static bool shouldSaveValue(const Attribute *);

  // should the property definition itself be saved, note this function must be true if the above is true
  // but the above can be false when this is true.
  static bool shouldSave(const Attribute *);

  // helper for custom saving, allows not saving input specifically.
  static void save(const Attribute *, Saver &, bool writeInput);

  // traits customised for each derived type
  template <typename T> struct TypeTraits
    {
    typedef PropertyBaseTypeTraits<T> Type;
    };
  };

class SHIFT_EXPORT PropertyContainerTraits : public PropertyBaseTraits
  {
public:
  static void assign(const Attribute *, Attribute *);
  static void save(const Attribute *, Saver & );
  static Attribute *load(Container *, Loader &);
  static bool shouldSaveValue(const Attribute *);
  };

template <typename T> class BasePODPropertyTraits : public PropertyBaseTraits
  {
public:
  static void save(const Attribute *p, Saver &l )
    {
    PropertyBaseTraits::save(p, l);
    }

  static Attribute *load(Container *parent, Loader &l)
    {
    Attribute *prop = PropertyBaseTraits::load(parent, l);
    return prop;
    }

  static bool shouldSaveValue(const Attribute *)
    {
    return false;
    }

  enum
    {
    assign = 0
    };
  };

template <typename T> class PODPropertyTraits : public BasePODPropertyTraits<T>
  {
public:
  static void save(const Attribute *p, Saver &l )
    {
    BasePODPropertyTraits<T>::save(p, l);
    const T *ptr = p->uncheckedCastTo<T>();
    writeValue(l, ptr->_value);
    }

  static Attribute *load(Container *parent, Loader &l)
    {
    Attribute *prop = BasePODPropertyTraits<T>::load(parent, l);
    T *ptr = prop->uncheckedCastTo<T>();
    readValue(l, ptr->_value);
    return prop;
    }

  static bool shouldSaveValue(const Attribute *p)
    {
    const T *ptr = p->uncheckedCastTo<T>();

    if(BasePODPropertyTraits<T>::shouldSaveValue(p))
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

  static void assign(const Shift::Attribute *p, Shift::Attribute *l)
    {
    T::assignBetween(p, l);
    }
  };

}
}

#endif // SPROPERTYTRAITS_H
