#ifndef SPROPERTYARRAY_H
#define SPROPERTYARRAY_H

#include "shift/Properties/spropertycontainer.h"
#include "shift/TypeInformation/styperegistry.h"

namespace Shift
{

class SHIFT_EXPORT PropertyArray : public PropertyContainer
  {
  S_PROPERTY_CONTAINER(PropertyArray, PropertyContainer, 0);

public:
  template <typename T> T *add(const PropertyNameArg& name)
    {
    return addProperty(T::staticTypeInformation(), X_SIZE_SENTINEL, name)->castTo<T>();
    }
  template <typename T> T *add(xsize index=X_SIZE_SENTINEL, const PropertyNameArg& name=PropertyNameArg())
    {
    return addProperty(T::staticTypeInformation(), index, name)->castTo<T>();
    }

  Property *add(const PropertyInformation *info, const PropertyNameArg &name, PropertyInstanceInformationInitialiser *init, xsize index=X_SIZE_SENTINEL)
    {
    return PropertyContainer::addProperty(info, index, name, init);
    }

  Property *add(const PropertyInformation *info, xsize index=X_SIZE_SENTINEL, const PropertyNameArg &name=PropertyNameArg())
    {
    return PropertyContainer::addProperty(info, index, name, 0);
    }

  Property *operator[](xsize i) { return at(i); }
  using PropertyContainer::at;
  using PropertyContainer::clear;

  void remove(Property *);
  };

template <typename T> class TypedPropertyArray : public PropertyContainer
  {
public:
  typedef T ElementType;

  ElementType *add()
    {
    return addProperty(T::staticTypeInformation())->castTo<T>();
    }

  void remove(Property *prop)
    {
    removeProperty(prop);
    }

  void resize(xsize s)
    {
    xsize sz = size();
    xsize dif = s - sz;

    if(dif > 0)
      {
      for(xsize x=0; x<s; ++x)
        {
        addProperty(T::staticTypeInformation());
        }
      }
    else if(dif < 0)
      {
      for(xsize x=s; x<sz; ++x)
        {
        // could be optimised lots...
        removeProperty(at(x));
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
  };

}

S_PROPERTY_INTERFACE(Shift::PropertyArray)

#endif // SPROPERTYARRAY_H
