#ifndef SCONTAINER_INL
#define SCONTAINER_INL

#include "shift/Properties/scontainer.h"

namespace Shift
{

template <typename T> const T *Container::firstDynamicChild() const
  {
  return ((Container*)this)->firstDynamicChild<T>();
  }

template <typename T> T *Container::firstDynamicChild()
  {
  Attribute *prop = firstDynamicChild();
  while(prop)
    {
    T *t = prop->castTo<T>();
    if(t)
      {
      return t;
      }
    prop = nextDynamicSibling(prop);
    }
  return 0;
  }

Attribute *Container::firstDynamicChild()
  {
  preGet();
  return _dynamicChild;
  }

const Attribute *Container::firstDynamicChild() const
  {
  preGet();
  return _dynamicChild;
  }

Attribute *Container::lastDynamicChild()
  {
  preGet();
  return _lastDynamicChild;
  }

const Attribute *Container::lastDynamicChild() const
  {
  preGet();
  return _lastDynamicChild;
  }

template <typename T> const T *Container::nextDynamicSibling(const T *old) const
  {
  return ((Container*)this)->nextDynamicSibling<T>((T*)old);
  }

template <typename T> T *Container::nextDynamicSibling(const T *old)
  {
  Attribute *prop = nextDynamicSibling((const Attribute*)old);
  while(prop)
    {
    T *t = prop->castTo<T>();
    if(t)
      {
      return t;
      }
    prop = nextDynamicSibling((const Attribute*)prop);
    }
  return 0;
  }

template <typename T> T *Container::findChild(const NameArg &name)
  {
  Attribute *prop = findChild(name);
  if(prop)
    {
    return prop->castTo<T>();
    }
  return 0;
  }

template <typename T> const T *Container::findChild(const NameArg &name) const
  {
  const Attribute *prop = findChild(name);
  if(prop)
    {
    return prop->castTo<T>();
    }
  return 0;
  }

}

#endif // SCONTAINER_INL
