#ifndef SPROPERTYCONTAINERITERATORS_H
#define SPROPERTYCONTAINERITERATORS_H

#include "shift/Properties/spropertycontainer.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/spropertyinstanceinformation.h"

namespace Shift
{

template <typename T, typename CONT> class PropertyContainerBaseIterator
  {
public:
  PropertyContainerBaseIterator(CONT *c, const PropertyInformation *i, xsize idx, T *dP)
    : _c(c),
      _info(i),
      _index(idx),
      _fromDynamic(dP)
    {
    xAssert(idx != X_SIZE_SENTINEL);
    _embeddedCount = _info->childCount();
    }
  T *operator*() const
    {
    if(_index < _embeddedCount)
      {
      const EmbeddedPropertyInstanceInformation *from = _info->childFromIndex(_index);
      Property *p = const_cast<Property *>(from->locateProperty(_c));
      xAssert(p->baseInstanceInformation());
      xAssert(from->childInformation() == p->typeInformation());
      return static_cast<T*>(p);
      }
    return _fromDynamic;
    }
  PropertyContainerBaseIterator<T,CONT>& operator++()
    {
    if(_index < _embeddedCount)
      {
      ++_index;
      }
    else if(_fromDynamic)
      {
      xAssert(_index == _embeddedCount);
      _fromDynamic = _c->nextDynamicSibling(_fromDynamic);
      }
    return *this;
    }
  bool operator!=(const PropertyContainerBaseIterator<T, CONT> &it) const
    {
    return _index != it._index || _fromDynamic != it._fromDynamic;
    }

protected:
  CONT *_c;
  const PropertyInformation *_info;
  xsize _index;
  xsize _embeddedCount;
  T *_fromDynamic;
  };

template <typename T, typename CONT> class PropertyContainerIterator : public PropertyContainerBaseIterator<T, CONT>
  {
public:
  PropertyContainerIterator(CONT *c, const PropertyInformation *i, xsize idx, T *dP)
    : PropertyContainerBaseIterator<T, CONT>(c, i, idx, dP)
    {
    }

  PropertyContainerBaseIterator<T,CONT>& operator++()
    {
    xsize &index = PropertyContainerBaseIterator<T, CONT>::_index;
    const xsize &lastIndex = PropertyContainerBaseIterator<T, CONT>::_embeddedCount;
    if(index < lastIndex)
      {
      typedef PropertyContainerBaseIterator<T, CONT> It;
      const PropertyInformation* info = It::_info;
      ++index;
      info->nextChild<T>(&index);
      }
    else
      {
      xAssert(index == lastIndex);
      T *&fromDynamic = PropertyContainerBaseIterator<T, CONT>::_fromDynamic;
      PropertyContainer *c = const_cast<PropertyContainer*>(PropertyContainerBaseIterator<T, CONT>::_c);
      fromDynamic = c->nextDynamicSibling<T>(fromDynamic);
      }
    return *this;
    }
  };


template <typename T, typename Cont, typename Iterator> class PropertyContainerTypedIteratorWrapperFrom
  {
  Cont *_cont;
  const PropertyInformation *_info;
  xsize _index;
  T *_fromDynamic;

public:
  PropertyContainerTypedIteratorWrapperFrom(Cont *cont,
                                             const PropertyInformation* info,
                                             xsize index,
                                             T* dynamicChild)
    : _cont(cont),
      _info(info),
      _index(index),
      _fromDynamic(dynamicChild)
    {
    }

  Iterator begin() { return Iterator(_cont, _info, _index, _fromDynamic); }
  Iterator end() { return Iterator(0, _info, xMin(0U, (xsize)_info->childCount()), 0); }
  };

#define WRAPPER_TYPE_FROM(T, CONT) PropertyContainerTypedIteratorWrapperFrom<T, CONT, PropertyContainerIterator<T, CONT> >
#define WRAPPER_TYPE_FROM_BASE(T, CONT) PropertyContainerTypedIteratorWrapperFrom<T, CONT, PropertyContainerBaseIterator<T, CONT> >

namespace detail
{
template <typename Res, typename T, typename Cont> Res makeWalker(Cont *c)
  {
  const PropertyInformation *info = c->typeInformation();

  xsize idx = 0;
  info->firstChild<T>(&idx);

  return Res(
      c,
      info,
      idx,
      c->firstDynamicChild<T>()
      );
  }

template <typename Res, typename Cont> Res makeTypelessWalker(Cont *c)
  {
  return Res(
      c,
      c->typeInformation(),
      0,
      c->firstDynamicChild()
      );
  }

template <typename Res, typename T, typename Cont>  Res makeWalkerFrom(Cont *c, T *prop)
  {
  xAssert(prop->parent() == c);
  xsize idx = 0;
  T *dyProp = 0;
  const PropertyInformation *info = c->typeInformation();

  if(!prop->isDynamic())
    {
    idx = prop->baseInstanceInformation()->index();
    dyProp = c->firstDynamicChild<T>();
    }
  else
    {
    idx = xMin(0U, (xsize)info->childCount());
    dyProp = prop;
    }


  return Res(
      c,
      info,
      idx,
      dyProp
      );
  }

template <typename Res, typename T, typename Cont> Res makeWalkerFromNext(Cont *c, T *prop)
  {
  xAssert(prop->parent() == c);
  xsize idx = 0;
  const T *dyProp = 0;
  const PropertyInformation* type = c->typeInformation();

  if(!prop->isDynamic())
    {
    idx = prop->baseInstanceInformation()->index();

    EmbeddedInstanceInformation* inst = type->childFromIndex(idx);
    while(inst && !inst->childInformation()->inheritsFromType(type))
      {
      ++idx;
      inst = type->childFromIndex(idx);
      }

    dyProp = firstDynamicChild<T>();
    }
  else
    {
    idx = xMin(0U, (xsize)type->childCount());
    Property* itProp = prop;
    while(!dyProp && itProp)
      {
      dyProp = itProp->castTo<T>();
      itProp = nextDynamicSibling(itProp);
      }
    }

  const PropertyInformation *info = c->typeInformation();

  return Res(
      c,
      info,
      idx,
      dyProp
      );
  }
}

inline WRAPPER_TYPE_FROM_BASE(Property, PropertyContainer) PropertyContainer::walkerFrom(Property *prop)
  {
  xAssert(prop->parent() == this);
  xsize idx = 0;
  Property *dyProp = 0;
  const PropertyInformation *info = typeInformation();

  if(!prop->isDynamic())
    {
    idx = prop->baseInstanceInformation()->index();
    dyProp = firstDynamicChild();
    }
  else
    {
    idx = xMin(0U, (xsize)info->childCount());
    dyProp = prop;
    }

  return WRAPPER_TYPE_FROM_BASE(Property, PropertyContainer)(
      this,
      info,
      idx,
      dyProp
      );
  }

template <typename T> WRAPPER_TYPE_FROM(T, PropertyContainer) PropertyContainer::walker()
  {
  return detail::makeWalker<WRAPPER_TYPE_FROM(T, PropertyContainer), T>(this);
  }

template <typename T> WRAPPER_TYPE_FROM(const T, const PropertyContainer) PropertyContainer::walker() const
  {
  return detail::makeWalker<WRAPPER_TYPE_FROM(const T, const PropertyContainer), T>(this);
  }

template <typename T> WRAPPER_TYPE_FROM(T, PropertyContainer) PropertyContainer::walkerFrom(T *prop)
  {
  return detail::makeWalkerFrom<WRAPPER_TYPE_FROM(T, PropertyContainer)>(this, prop);
  }

template <typename T> WRAPPER_TYPE_FROM(const T, const PropertyContainer)
    PropertyContainer::walkerFrom(const T *prop) const
  {
  return detail::makeWalkerFrom<WRAPPER_TYPE_FROM(const T, const PropertyContainer)>(this, prop);
  }

template <typename T> WRAPPER_TYPE_FROM(T, PropertyContainer) PropertyContainer::walkerFrom(Property *prop)
  {
  return detail::makeWalkerFromNext<WRAPPER_TYPE_FROM(T, PropertyContainer)>(this, p);
  }

template <typename T> WRAPPER_TYPE_FROM(const T, const PropertyContainer) PropertyContainer::walkerFrom(const Property *prop) const
  {
  return detail::makeWalkerFromNext<WRAPPER_TYPE_FROM(const T, const PropertyContainer)>(this, p);
  }

inline WRAPPER_TYPE_FROM_BASE(Property, PropertyContainer) PropertyContainer::walker()
  {
  return detail::makeTypelessWalker<WRAPPER_TYPE_FROM_BASE(Property, PropertyContainer)>(this);
  }

inline WRAPPER_TYPE_FROM_BASE(const Property, const PropertyContainer) PropertyContainer::walker() const
  {
  return detail::makeTypelessWalker<WRAPPER_TYPE_FROM_BASE(const Property, const PropertyContainer)>(this);
  }

#undef WRAPPER_TYPE_FROM
#undef WRAPPER_TYPE_FROM_BASE

}

#endif // SPROPERTYCONTAINERITERATORS_H
