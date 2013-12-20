#ifndef SPROPERTYTRAITS_H
#define SPROPERTYTRAITS_H

#include "spropertyinformation.h"

namespace Shift
{

template <typename TypeTraitsCreation, bool Abstract> class PropertyCreateSelector
  {
public:
  static Attribute *create(const Eks::MemoryResource &ptr)
    {
    return TypeTraitsCreation::create(ptr);
    }
  static Eks::MemoryResource destroy(Attribute *ptr)
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

    fns.createEmbeddedInstanceInformation = TypeTraits::createEmbeddedInstanceInformation;
    fns.createDynamicInstanceInformation = TypeTraits::createDynamicInstanceInformation;
    fns.destroyEmbeddedInstanceInformation = TypeTraits::destroyEmbeddedInstanceInformation;
    fns.destroyDynamicInstanceInformation = TypeTraits::destroyDynamicInstanceInformation;

    fns.shouldSave = Traits::shouldSave;
    fns.shouldSaveValue = Traits::shouldSaveValue;
    fns.save = (PropertyInformationFunctions::SaveFunction)Traits::save;
    fns.load = (PropertyInformationFunctions::LoadFunction)Traits::load;
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
    static Attribute *create(const Eks::MemoryResource &ptr)
      {
      Type *t = ptr.create<Type>();
      xAssert(t == ptr.data());

      return t;
      }
    static Eks::MemoryResource destroy(Attribute *ptr)
      {
      return Eks::MemoryResource::destroy((Type*)ptr);
      }
    };

  static PropertyInstanceInformation *createDynamicInstanceInformation(
      const Eks::MemoryResource &allocation,
      const PropertyInstanceInformation *cpy)
    {
    if(cpy)
      {
      return allocation.create<DyInst>(*static_cast<const DyInst*>(cpy));
      }

    return allocation.create<DyInst>();
    }
  static PropertyInstanceInformation *createEmbeddedInstanceInformation(
      const Eks::MemoryResource &allocation,
      const PropertyInstanceInformation *cpy)
    {
    if(cpy)
      {
      return allocation.create<StInst>(*static_cast<const StInst*>(cpy));
      }

    return allocation.create<StInst>();
    }
  static Eks::MemoryResource destroyDynamicInstanceInformation(PropertyInstanceInformation *allocation)
    {
    return Eks::MemoryResource::destroy((DyInst*)allocation);
    }
  static Eks::MemoryResource destroyEmbeddedInstanceInformation(PropertyInstanceInformation *allocation)
    {
    return Eks::MemoryResource::destroy((StInst*)allocation);
    }
  };

class SHIFT_EXPORT PropertyBaseTraits
  {
public:
  static void assign(const Attribute *, Attribute *);
  static void save(const Attribute *, AttributeSaver & );
  static Attribute *load(Container *, AttributeLoader &);

  // should this properties value be saved, for example not when the value
  // is this property's value the default as it is when created.
  static bool shouldSaveValue(const Attribute *);

  // should the property definition itself be saved, note this function must be true if the above is true
  // but the above can be false when this is true.
  static bool shouldSave(const Attribute *);

  // helper for custom saving, allows not saving input specifically.
  static void save(const Attribute *, AttributeSaver &, bool writeInput);

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
  static bool shouldSaveValue(const Attribute *);
  };

}
}

#endif // SPROPERTYTRAITS_H
