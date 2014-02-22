#ifndef SPROPERTYARRAY_H
#define SPROPERTYARRAY_H

#include "shift/Properties/scontainer.h"
#include "shift/TypeInformation/styperegistry.h"

namespace Shift
{

class SHIFT_EXPORT Array : public Container
  {
  S_PROPERTY_CONTAINER(Array, Container);

public:
  enum
    {
    DynamicChildMode = Container::DynamicChildMode | Shift::IndexedChildren
    };

  template <typename T> T *add(xsize index=std::numeric_limits<xsize>::max())
    {
    return addAttribute(T::staticTypeInformation(), index, NameArg())->template castTo<T>();
    }

  Attribute *add(const PropertyInformation *info, PropertyInstanceInformationInitialiser *init=0, xsize index=std::numeric_limits<xsize>::max())
    {
    return Container::addAttribute(info, index, NameArg(), init);
    }

  Attribute *add(const PropertyInformation *info, xsize index=std::numeric_limits<xsize>::max())
    {
    return Container::addAttribute(info, index, NameArg(), 0);
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

S_PROPERTY_INTERFACE(Shift::Array)

#endif // SPROPERTYARRAY_H
