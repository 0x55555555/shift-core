#ifndef SPROPERTYARRAY_H
#define SPROPERTYARRAY_H

#include "shift/Properties/scontainer.h"
#include "shift/TypeInformation/styperegistry.h"

namespace Shift
{

class SHIFT_EXPORT Array : public Container
  {
  S_PROPERTY_CONTAINER(Array, Container, 0);

public:
  template <typename T> T *add(const NameArg& name)
    {
    return addProperty(T::staticTypeInformation(), X_SIZE_SENTINEL, name)->castTo<T>();
    }
  template <typename T> T *add(xsize index=X_SIZE_SENTINEL, const NameArg &name=NameArg())
    {
    return addAttribute(T::staticTypeInformation(), index, name)->castTo<T>();
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
    return addAttribute(T::staticTypeInformation())->castTo<T>();
    }

  void remove(Attribute *prop)
    {
    removeAttribute(prop);
    }

  void resize(xsize s)
    {
    xsize sz = size();
    xsize dif = s - sz;

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

S_PROPERTY_INTERFACE(Shift::Array)

#endif // SPROPERTYARRAY_H
