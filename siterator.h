#ifndef SITERATOR_H
#define SITERATOR_H

#include "spropertycontainer.h"

#define S_ITERATOR_STACK_SIZE 8

class SHIFT_EXPORT SIterator
  {
public:
  class DataCache
    {
  public:
    DataCache(SProperty *input)
      {
      reset(input);
      }

    inline void setOutput(xsize index, SProperty *in)
      {
      xAssert(index < S_ITERATOR_STACK_SIZE);
      ptr[index] = in;
      }

    inline SProperty *output(xsize index)
      {
      xAssert(index < S_ITERATOR_STACK_SIZE);
      return ptr[index];
      }

    inline void reset(SProperty *prop)
      {
      ptr[0] = prop;
      for(xsize i=1; i<S_ITERATOR_STACK_SIZE; ++i)
        {
        ptr[i] = 0;
        }
      }

  private:
    SProperty *ptr[S_ITERATOR_STACK_SIZE];
    };

  typedef SProperty *(*FilterFunction)(DataCache &previous);

XProperties:
  XProperty(FilterFunction, filter, setFilter);
  XProperty(SProperty *, property, setProperty);

public:
  template<template<SIterator::FilterFunction Z, xsize INDEX1> class A>
      static SIterator::FilterFunction createFilter()
    {
    return A<Terminate<0>, 1>::filter;
    }

  template<template<SIterator::FilterFunction Z, xsize INDEX1> class A,
      template<SIterator::FilterFunction Y, xsize INDEX2> class B>
      static SIterator::FilterFunction createFilter()
    {
    return B<A<Terminate<0>, 1>::filter, 2>::filter;
    }

  // lambda support
  template <typename T> void each(T t)
    {
    xAssert(filter());

    DataCache cache(property());
    while(SProperty *prop = filter()(cache))
      {
      t(prop);
      }
    }

private:
  template <xsize INDEX> static inline SProperty *Terminate(SIterator::DataCache &cache)
    {
    SProperty *input = cache.output(INDEX);
    cache.setOutput(INDEX, 0);
    return input;
    }
  };

template <SIterator::FilterFunction CHILD, xsize INDEX>
class ChildTree
  {
public:
    static inline SProperty *filter(SIterator::DataCache &cache)
    {
    SProperty *previous = (SProperty *)cache.output(INDEX);
    SProperty *ret = 0;
    if(!previous)
      {
      ret = CHILD(cache);
      }
    else
      {
      SPropertyContainer *cont = previous->castTo<SPropertyContainer>();
      if(cont && cont->firstChild())
        {
        ret = cont->firstChild();
        }
      else if(previous != cache.output(INDEX-1) && previous->nextSibling())
        {
        ret = previous->nextSibling();
        }
      else
        {
        // iterate up until we reach the start point or an unvisited sibling.
        SProperty *parent = previous->parent();
        while(parent && !ret && parent != cache.output(INDEX-1))
          {
          ret = parent->nextSibling();
          parent = parent->parent();
          }

        if(!ret)
          {
          ret = CHILD(cache);
          }
        }
      }

    cache.setOutput(INDEX, ret);
    return ret;
    }
  };

template <SIterator::FilterFunction CHILD, xsize INDEX>
class DirectChildren
  {
public:
    static inline SProperty *filter(SIterator::DataCache &cache)
    {
    SProperty *previous = (SProperty *)cache.output(INDEX);
    SProperty *ret = 0;

    if(!previous || !previous->nextSibling())
      {
      SProperty *prop = CHILD(cache);
      if(prop)
        {
        SPropertyContainer *cont = prop->castTo<SPropertyContainer>();
        if(cont)
          {
          ret = cont->firstChild();
          }
        }
      }
    else
      {
      ret = previous->nextSibling();
      }

    cache.setOutput(INDEX, ret);
    return ret;
    }
  };

template <SIterator::FilterFunction CHILD, xsize INDEX>
class DirectEntityChildren
  {
public:
    static inline SProperty *filter(SIterator::DataCache &cache)
    {
    SProperty *previous = (SProperty *)cache.output(INDEX);
    SProperty *ret = 0;

    if(!previous || !previous->nextSibling())
      {
      SProperty *prop = CHILD(cache);
      if(prop)
        {
        SEntity *ent = prop->castTo<SEntity>();
        if(ent)
          {
          ret = ent->children.firstChild();
          }
        }
      }
    else
      {
      ret = previous->nextSibling();
      }

    cache.setOutput(INDEX, ret);
    return ret;
    }
  };

template <typename TYPE> class Typed
  {
public:
  template <SIterator::FilterFunction CHILD, xsize INDEX> class Is
    {
  public:
    static inline SProperty *filter(SIterator::DataCache &cache)
      {
      SProperty *child = CHILD(cache);

      while(child && !child->castTo<TYPE>())
        {
        child = CHILD(cache);
        }

      cache.setOutput(INDEX, child);
      return child;
      }
    };
  template <SIterator::FilterFunction CHILD, xsize INDEX> class IsNot
    {
  public:
    static inline SProperty *filter(SIterator::DataCache &cache)
      {
      SProperty *child = CHILD(cache);

      while(child && child->castTo<TYPE>())
        {
        child = CHILD(cache);
        }

      cache.setOutput(INDEX, child);
      return child;
      }
    };
  };

template <SIterator::FilterFunction CHILD, xsize INDEX> class Parent
  {
public:
  static inline SProperty *filter(SIterator::DataCache &cache)
    {
    SProperty *ret = CHILD(cache);
    if(ret)
      {
      ret = ret->parent();
      }

    cache.setOutput(INDEX, ret);
    return ret;
    }
  };

#endif // SITERATOR_H
