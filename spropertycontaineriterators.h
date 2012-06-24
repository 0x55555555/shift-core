#ifndef SPROPERTYCONTAINERITERATORS_H
#define SPROPERTYCONTAINERITERATORS_H

#include "spropertyinformation.h"

template <typename T, typename CONT> class SPropertyContainerBaseIterator
  {
  CONT *_c;
  const SPropertyInstanceInformation *_from;
  T *_fromDynamic;
public:
  SPropertyContainerBaseIterator(CONT *c, const SPropertyInstanceInformation *p, T *dP) : _c(c), _from(p), _fromDynamic(dP) { }
  T *operator*() const
    {
    if(_from)
      {
      return _from->locateProperty(_c);
      }
    return _fromDynamic;
    }
  void operator++()
    {
    if(_from)
      {
      SPropertyInstanceInformation *nextStatic = _from->nextSibling();
      _from = nextStatic;
      }
    else
      {
      _fromDynamic = _c->nextDynamicSibling(_fromDynamic);
      }
    }
  bool operator!=(const SPropertyContainerBaseIterator<T, CONT> &it) const { return _from != it._from || _fromDynamic != it._fromDynamic; }
  };

template <typename T, typename CONT> class SPropertyContainerIterator
  {
  CONT *_c;
  const SPropertyInstanceInformation *_from;
  T *_fromDynamic;
public:
  SPropertyContainerIterator(CONT *c, const SPropertyInstanceInformation *p, T *dP) : _c(c), _from(p), _fromDynamic(_dP) { }
  //T *operator*() const { return _p; }
  //void operator++() { _p = _c->nextSibling<T>(_p); }
  bool operator!=(const SPropertyContainerBaseIterator<T, CONT> &it) const { return _from != it._from || _fromDynamic != it._fromDynamic; }
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


template <typename T>
    SPropertyContainerTypedIteratorWrapperFrom<T, SPropertyContainer, SPropertyContainerIterator<T, SPropertyContainer> > SPropertyContainer::walker()
  {
  return SPropertyContainerTypedIteratorWrapperFrom<T, SPropertyContainer, SPropertyContainerIterator<T, SPropertyContainer>>(
        this,
        typeInformation()->firstChild<T>(),
        firstDynamicChild<T>()
        );
  }

template <typename T>
    SPropertyContainerTypedIteratorWrapperFrom<const T, const SPropertyContainer, SPropertyContainerIterator<const T, const SPropertyContainer> > SPropertyContainer::walker() const
  {
  return SPropertyContainerTypedIteratorWrapperFrom<const T, const SPropertyContainer, SPropertyContainerIterator<const T, const SPropertyContainer>>(
        this,
        typeInformation()->firstChild<T>(),
        firstDynamicChild<T>()
        );
  }

SPropertyContainerTypedIteratorWrapperFrom<SProperty, SPropertyContainer, SPropertyContainerBaseIterator<SProperty, SPropertyContainer> > SPropertyContainer::walker()
  {
  return SPropertyContainerTypedIteratorWrapperFrom<SProperty, SPropertyContainer, SPropertyContainerBaseIterator<SProperty, SPropertyContainer>>(
        this,
        typeInformation()->firstChild(),
        firstDynamicChild()
        );
  }

SPropertyContainerTypedIteratorWrapperFrom<const SProperty, const SPropertyContainer, SPropertyContainerBaseIterator<const SProperty, const SPropertyContainer> > SPropertyContainer::walker() const
  {
  return SPropertyContainerTypedIteratorWrapperFrom<const SProperty, const SPropertyContainer, SPropertyContainerBaseIterator<const SProperty, const SPropertyContainer> >(
        this,
        typeInformation()->firstChild(),
        firstDynamicChild()
        );
  }

SPropertyContainerTypedIteratorWrapperFrom<SProperty, SPropertyContainer, SPropertyContainerBaseIterator<SProperty, SPropertyContainer> > SPropertyContainer::walkerFrom(SProperty *prop)
  {
  if(!prop->isDynamic())
    {
    return SPropertyContainerTypedIteratorWrapperFrom<SProperty, SPropertyContainer, SPropertyContainerBaseIterator<SProperty, SPropertyContainer>>(
          this,
          prop->instanceInformation(),
          firstDynamicChild()
          );
    }
  else
    {
    return SPropertyContainerTypedIteratorWrapperFrom<SProperty, SPropertyContainer, SPropertyContainerBaseIterator<SProperty, SPropertyContainer>>(
          this,
          0,
          prop
          );
    }
  }


#endif // SPROPERTYCONTAINERITERATORS_H
