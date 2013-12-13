#ifndef SPROPERTYARRAY_H
#define SPROPERTYARRAY_H

#include "shift/Properties/scontainer.h"
#include "shift/TypeInformation/styperegistry.h"

namespace Shift
{

class SHIFT_EXPORT Set : public Container
  {
  S_PROPERTY_CONTAINER(Set, Container);

public:
  enum
    {
    DynamicChildMode = Container::DynamicChildMode | Shift::NamedChildren
    };

  template <typename T> T *add(const NameArg& name)
    {
    return addAttribute(T::staticTypeInformation(), X_SIZE_SENTINEL, name)->template castTo<T>();
    }
  template <typename T> T *add(xsize index=X_SIZE_SENTINEL, const NameArg &name=NameArg())
    {
    return addAttribute(T::staticTypeInformation(), index, name)->template castTo<T>();
    }

  Attribute *add(const PropertyInformation *info, const NameArg &name, PropertyInstanceInformationInitialiser *init=0, xsize index=X_UINT8_SENTINEL)
    {
    return Container::addAttribute(info, index, name, init);
    }

  Attribute *add(const PropertyInformation *info, xsize index=X_SIZE_SENTINEL, const NameArg &name=NameArg())
    {
    return Container::addAttribute(info, index, name, 0);
    }

  Attribute *operator[](xsize i) { return at(i); }
  using Container::at;
  using Container::clear;

  void remove(Attribute *);
  };

template <typename T> class TypedPropertyArray : public Container
  {
public:
  typedef T ElementType;

  ElementType *add()
    {
    return addAttribute(T::staticTypeInformation())->template castTo<T>();
    }

  void remove(Attribute *prop)
    {
    removeAttribute(prop);
    }

  void resize(xsize s)
    {
    xsize sz = size();
    xptrdiff dif = s - sz;

    if(dif > 0)
      {
      for(xsize x=0; x<s; ++x)
        {
        addAttribute(T::staticTypeInformation());
        }
      }
    else if(dif < 0)
      {
      for(xsize x=s; x<sz; ++x)
        {
        // could be optimised lots...
        removeAttribute(at(x));
        }
      }
    }

  ElementType *operator[](xsize i) { return at(i); }
  ElementType *at(xsize i)
    {
    xForeach(auto child, walker<ElementType>())
      {
      if(i == 0)
        {
        return child;
        }
      --i;
      }

    return 0;
    }

  const ElementType *operator[](xsize i) const { return at(i); }
  const ElementType *at(xsize i) const
    {
    xForeach(auto child, walker<ElementType>())
      {
      if(i == 0)
        {
        return child;
        }
      --i;
      }

    return 0;
    }
  };

}

S_PROPERTY_INTERFACE(Shift::Set)

#endif // SPROPERTYARRAY_H
