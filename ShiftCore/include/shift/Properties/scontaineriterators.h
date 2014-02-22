#ifndef SContainerITERATORS_H
#define SContainerITERATORS_H

#include "shift/Properties/scontainer.h"
#include "shift/Properties/scontainer.inl"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/spropertyinstanceinformation.h"

namespace Shift
{

template <typename T, typename CONT> class ContainerBaseIterator
  {
public:
  ContainerBaseIterator()
    : _c(0),
      _info(0),
      _index(0),
      _fromDynamic(0)
    {
    }

  ContainerBaseIterator(CONT *c, const PropertyInformation *i, xsize idx, T *dP)
    : _c(c),
      _info(i),
      _index(idx),
      _fromDynamic(dP)
    {
    xAssert(idx != Eks::maxFor(idx));
    _embeddedCount = _info->childCount();
    }
  T *operator*() const
    {
    if(_index < _embeddedCount)
      {
      const EmbeddedPropertyInstanceInformation *from = _info->childFromIndex(_index);
      Attribute *p = const_cast<Attribute *>(from->locate(_c));
      xAssert(p->baseInstanceInformation());
      xAssert(from->childInformation() == p->typeInformation());
      return static_cast<T*>(p);
      }
    return _fromDynamic;
    }
  ContainerBaseIterator<T,CONT>& operator++()
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
  bool operator!=(const ContainerBaseIterator<T, CONT> &it) const
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

template <typename T, typename CONT> class ContainerIterator : public ContainerBaseIterator<T, CONT>
  {
public:
  ContainerIterator()
    {
    }

  ContainerIterator(CONT *c, const PropertyInformation *i, xsize idx, T *dP)
    : ContainerBaseIterator<T, CONT>(c, i, idx, dP)
    {
    }

  ContainerBaseIterator<T,CONT>& operator++()
    {
    xsize &index = ContainerBaseIterator<T, CONT>::_index;
    const xsize &lastIndex = ContainerBaseIterator<T, CONT>::_embeddedCount;
    if(index < lastIndex)
      {
      typedef ContainerBaseIterator<T, CONT> It;
      const PropertyInformation* info = It::_info;
      ++index;
      info->nextChild<T>(&index);
      if(index == Eks::maxFor(index))
        {
        index = lastIndex;
        }
      }
    else
      {
      xAssert(index == lastIndex);
      T *&fromDynamic = ContainerBaseIterator<T, CONT>::_fromDynamic;
      Container *c = const_cast<Container*>(ContainerBaseIterator<T, CONT>::_c);
      fromDynamic = c->nextDynamicSibling<T>(fromDynamic);
      }
    return *this;
    }
  };


template <typename T, typename Cont, typename _Iterator> class ContainerTypedIteratorWrapperFrom
  {
  Cont *_cont;
  const PropertyInformation *_info;
  xsize _index;
  T *_fromDynamic;

public:
  typedef _Iterator Iterator;

  ContainerTypedIteratorWrapperFrom()
    : _cont(0),
      _info(0),
      _index(0),
      _fromDynamic(0)
    {
    }

  ContainerTypedIteratorWrapperFrom(Cont *cont,
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
  Iterator end()
    {
    xsize lastChildIndex = _info->childCount();
    return Iterator(0, _info, lastChildIndex, 0);
    }
  };

#define WRAPPER_TYPE_FROM(T, CONT) ContainerTypedIteratorWrapperFrom<T, CONT, ContainerIterator<T, CONT> >
#define WRAPPER_TYPE_FROM_BASE(T, CONT) ContainerTypedIteratorWrapperFrom<T, CONT, ContainerBaseIterator<T, CONT> >

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
      c->template firstDynamicChild<T>()
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
    idx = prop->embeddedInstanceInformation()->index();
    dyProp = c->template firstDynamicChild<T>();
    }
  else
    {
    idx = (xsize)info->childCount();
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
    idx = prop->embeddedInstanceInformation()->index();

    const EmbeddedPropertyInstanceInformation* inst = type->childFromIndex(idx);
    while(inst && !inst->childInformation()->inheritsFromType(type))
      {
      ++idx;
      inst = type->childFromIndex(idx);
      }

    dyProp = c->template firstDynamicChild<T>();
    }
  else
    {
    idx = (xsize)type->childCount();
    Attribute* itProp = prop;
    while(!dyProp && itProp)
      {
      dyProp = itProp->castTo<T>();
      itProp = c->nextDynamicSibling(itProp);
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

inline Container::Walker Container::walkerFrom(Attribute *prop)
  {
  xAssert(prop->parent() == this);
  xuint8 idx = 0;
  Attribute *dyProp = 0;
  const PropertyInformation *info = typeInformation();

  if(!prop->isDynamic())
    {
    idx = prop->embeddedInstanceInformation()->index();
    dyProp = firstDynamicChild();
    }
  else
    {
    idx = info->childCount();
    dyProp = prop;
    }

  return Container::Walker(
      this,
      info,
      idx,
      dyProp
      );
  }

template <typename T> WRAPPER_TYPE_FROM(T, Container) Container::walker()
  {
  return detail::makeWalker<WRAPPER_TYPE_FROM(T, Container), T>(this);
  }

template <typename T> WRAPPER_TYPE_FROM(const T, const Container) Container::walker() const
  {
  return detail::makeWalker<WRAPPER_TYPE_FROM(const T, const Container), T>(this);
  }

template <typename T> WRAPPER_TYPE_FROM(T, Container) Container::walkerFromTyped(T *prop)
  {
  return detail::makeWalkerFrom<WRAPPER_TYPE_FROM(T, Container)>(this, prop);
  }

template <typename T> WRAPPER_TYPE_FROM(const T, const Container)
    Container::walkerFromTyped(const T *prop) const
  {
  return detail::makeWalkerFrom<WRAPPER_TYPE_FROM(const T, const Container)>(this, prop);
  }

template <typename T> WRAPPER_TYPE_FROM(T, Container) Container::walkerFromTyped(Attribute *prop)
  {
  return detail::makeWalkerFromNext<WRAPPER_TYPE_FROM(T, Container)>(this, prop);
  }

template <typename T> WRAPPER_TYPE_FROM(const T, const Container) Container::walkerFromTyped(const Attribute *prop) const
  {
  return detail::makeWalkerFromNext<WRAPPER_TYPE_FROM(const T, const Container)>(this, prop);
  }

inline Container::Walker Container::walker()
  {
  return detail::makeTypelessWalker<Container::Walker>(this);
  }

inline Container::ConstWalker Container::walker() const
  {
  return detail::makeTypelessWalker<Container::ConstWalker>(this);
  }

#undef WRAPPER_TYPE_FROM
#undef WRAPPER_TYPE_FROM_BASE

}

#endif // SContainerITERATORS_H
