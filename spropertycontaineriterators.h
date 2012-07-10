#ifndef SPROPERTYCONTAINERITERATORS_H
#define SPROPERTYCONTAINERITERATORS_H

#include "spropertyinformation.h"

template <typename T, typename CONT> class SPropertyContainerBaseIterator
  {
public:
  SPropertyContainerBaseIterator(CONT *c, const SPropertyInstanceInformation *p, T *dP)
    : _c(c), _from(p), _fromDynamic(dP)
    {
    }
  T *operator*() const
    {
    if(_from)
      {
      return static_cast<T*>(_from->locateProperty(_c));
      }
    return _fromDynamic;
    }
  SPropertyContainerBaseIterator<T,CONT>& operator++()
    {
    if(_from)
      {
      const SPropertyInstanceInformation *nextStatic = _from->nextSibling();
      _from = nextStatic;
      }
    else
      {
      _fromDynamic = _c->nextDynamicSibling(_fromDynamic);
      }
    return *this;
    }
  bool operator!=(const SPropertyContainerBaseIterator<T, CONT> &it) const { return _from != it._from || _fromDynamic != it._fromDynamic; }
protected:
  CONT *_c;
  const SPropertyInstanceInformation *_from;
  T *_fromDynamic;
  };

template <typename T, typename CONT> class SPropertyContainerIterator : public SPropertyContainerBaseIterator<T, CONT>
  {
public:
  SPropertyContainerIterator(CONT *c, const SPropertyInstanceInformation *p, T *dP)
    : SPropertyContainerBaseIterator(c, p, dP)
    {
    }
  SPropertyContainerIterator<T,CONT>& operator++()
    {
    if(_from)
      {
      const SPropertyInstanceInformation *nextStatic = _from->nextSibling<T>();
      _from = nextStatic;
      }
    else
      {
      _fromDynamic = _c->nextDynamicSibling(_fromDynamic);
      }
    return *this;
    }
  };


template <typename T, typename Cont, typename Iterator> class SPropertyContainerTypedIteratorWrapperFrom
  {
  Cont *_cont;
  const SPropertyInstanceInformation *_from;
  T *_fromDynamic;
public:

  SPropertyContainerTypedIteratorWrapperFrom(Cont *cont, const SPropertyInstanceInformation* firstChild, T* dynamicChild) : _cont(cont), _from(firstChild), _fromDynamic(dynamicChild) { }
  Iterator begin() { return Iterator(_cont, _from, _fromDynamic); }
  Iterator end() { return Iterator(0, 0, 0); }
  };

#define WRAPPER_TYPE_FROM(T, CONT) SPropertyContainerTypedIteratorWrapperFrom<T, CONT, SPropertyContainerIterator<T, CONT> >
#define WRAPPER_TYPE_FROM_BASE(T, CONT) SPropertyContainerTypedIteratorWrapperFrom<T, CONT, SPropertyContainerBaseIterator<T, CONT> >

template <typename T> WRAPPER_TYPE_FROM(T, SPropertyContainer) SPropertyContainer::walker()
  {
  return WRAPPER_TYPE_FROM(T, SPropertyContainer)(
        this,
        typeInformation()->firstChild<T>(),
        firstDynamicChild<T>()
        );
  }

template <typename T> WRAPPER_TYPE_FROM(const T, const SPropertyContainer) SPropertyContainer::walker() const
  {
  return WRAPPER_TYPE_FROM(const T, const SPropertyContainer)(
        this,
        typeInformation()->firstChild<T>(),
        firstDynamicChild<T>()
        );
  }

template <typename T> WRAPPER_TYPE_FROM(T, SPropertyContainer) SPropertyContainer::walkerFrom(T *prop)
  {
  const SPropertyInstanceInformation *inst = 0;
  T *dyProp = 0;
  if(!prop->isDynamic())
    {
    inst = prop->instanceInformation();
    dyProp = firstDynamicChild<T>();
    }
  else
    {
    inst = 0;
    dyProp = prop;
    }

  return WRAPPER_TYPE_FROM(T, SPropertyContainer)(
      this,
      inst,
      dyProp
      );
  }

template <typename T> WRAPPER_TYPE_FROM(const T, const SPropertyContainer) SPropertyContainer::walkerFrom(const T *prop) const
  {
  const SPropertyInstanceInformation *inst = 0;
  const T *dyProp = 0;
  if(!prop->isDynamic())
    {
    inst = prop->instanceInformation();
    dyProp = firstDynamicChild<T>();
    }
  else
    {
    inst = 0;
    dyProp = prop;
    }

  return WRAPPER_TYPE_FROM(const T, const SPropertyContainer)(
      this,
      inst,
      dyProp
      );
  }

template <typename T> WRAPPER_TYPE_FROM(T, SPropertyContainer) SPropertyContainer::walkerFrom(SProperty *prop)
  {
  SPropertyInstanceInformation *inst = 0;
  const T *dyProp = 0;
  if(!prop->isDynamic())
    {
    inst = prop->instanceInformation();
    const SPropertyInformation* type = T::staticTypeInformation();
    while(inst && !inst->childInformation()->inheritsFromType(type))
      {
      inst = inst->nextSibling();
      }

    dyProp = firstDynamicChild<T>();
    }
  else
    {
    inst = 0;
    SProperty* itProp = prop;
    while(!dyProp && itProp)
      {
      dyProp = itProp->castTo<T>();
      itProp = nextDynamicSibling(itProp);
      }
    }

  return WRAPPER_TYPE_FROM(T, SPropertyContainer)(
      this,
      inst,
      dyProp
      );
  }

template <typename T> WRAPPER_TYPE_FROM(const T, const SPropertyContainer) SPropertyContainer::walkerFrom(const SProperty *prop) const
  {
  const SPropertyInstanceInformation *inst = 0;
  const T *dyProp = 0;
  if(!prop->isDynamic())
    {
    inst = prop->instanceInformation();
    const SPropertyInformation* type = T::staticTypeInformation();
    while(inst && !inst->childInformation()->inheritsFromType(type))
      {
      inst = inst->nextSibling();
      }

    dyProp = firstDynamicChild<T>();
    }
  else
    {
    inst = 0;
    const SProperty* itProp = prop;
    while(!dyProp && itProp)
      {
      dyProp = itProp->castTo<T>();
      itProp = nextDynamicSibling(itProp);
      }
    }

  return WRAPPER_TYPE_FROM(const T, const SPropertyContainer)(
      this,
      inst,
      dyProp
      );
  }

inline WRAPPER_TYPE_FROM_BASE(SProperty, SPropertyContainer) SPropertyContainer::walker()
  {
  return WRAPPER_TYPE_FROM_BASE(SProperty, SPropertyContainer)(
        this,
        typeInformation()->firstChild(),
        firstDynamicChild()
        );
  }

inline WRAPPER_TYPE_FROM_BASE(const SProperty, const SPropertyContainer) SPropertyContainer::walker() const
  {
  return WRAPPER_TYPE_FROM_BASE(const SProperty, const SPropertyContainer)(
        this,
        typeInformation()->firstChild(),
        firstDynamicChild()
        );
  }

inline WRAPPER_TYPE_FROM_BASE(SProperty, SPropertyContainer) SPropertyContainer::walkerFrom(SProperty *prop)
  {
  const SPropertyInstanceInformation *inst = 0;
  SProperty *dyProp = 0;
  if(!prop->isDynamic())
    {
    inst = prop->instanceInformation();
    dyProp = firstDynamicChild();
    }
  else
    {
    inst = 0;
    dyProp = prop;
    }

  return WRAPPER_TYPE_FROM_BASE(SProperty, SPropertyContainer)(
      this,
      inst,
      dyProp
      );
  }

#undef WRAPPER_TYPE_FROM
#undef WRAPPER_TYPE_FROM_BASE


#endif // SPROPERTYCONTAINERITERATORS_H
