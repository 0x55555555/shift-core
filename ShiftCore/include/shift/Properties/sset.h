#ifndef SPROPERTYSET_H
#define SPROPERTYSET_H

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

}

S_PROPERTY_INTERFACE(Shift::Set)

#endif // SPROPERTYSET_H
