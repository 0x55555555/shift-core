#ifndef SITERATOR_H
#define SITERATOR_H

#include "sentity.h"

namespace SIterator
{
template <typename IteratorType, typename ReturnTypeIn, typename DerivedExtraData> class Base
  {
public:
  typedef DerivedExtraData ExtraData;
  typedef ReturnTypeIn ReturnType;

  inline Base() : _prop(0)
    {
    }

  inline void reset(SProperty *prop)
    {
    _prop = prop;
    }

  inline SProperty *property() const
    {
    return _prop;
    }

  class IteratorBase
    {
  public:
    inline IteratorBase()
      {
      }

    inline IteratorBase(ReturnType *p) : _property(p)
      {
      }

    inline ReturnType *operator*() const
      {
      return _property;
      }

    inline ReturnType *operator->() const
      {
      return _property;
      }

    inline void setProperty(ReturnType *prop)
      {
      _property = prop;
      }

    inline bool operator!=(const IteratorBase& it) const
      {
      return _property != it._property;
      }

  private:
    ReturnType *_property;
    };

  class Iterator : public IteratorBase
    {
  public:
    inline Iterator()
      {
      }

    inline Iterator(ReturnType *p) : IteratorBase(p)
      {
      }

    inline Iterator(ReturnType *p, const ExtraData& d)
        : IteratorBase(p),
          _extra(d)
      {
      }

    inline void operator++()
      {
      IteratorType::next(*this);
      }

    inline void operator++(int)
      {
      IteratorType::next(*this);
      }

    inline ExtraData& data()
      {
      return _extra;
      }

  private:
    ExtraData _extra;
    };

  inline Iterator begin()
    {
    Iterator i;
    static_cast<IteratorType*>(this)->first(i);
    return i;
    }

  inline Iterator end()
    {
    return Iterator(0);
    }

private:
  SProperty *_prop;
  };

struct NilExtraData
  {
  };

template <typename ToForward, typename ParentType> struct ForwarderExtraData
  {
  inline ForwarderExtraData()
    {
    }

  inline ForwarderExtraData(const typename ParentType::Iterator& i) : _parent(i)
    {
    }

  ToForward _fwd;
  typename ToForward::ExtraData _fwdData;
  typename ParentType::Iterator _parent;
  };

template <typename ToForward, typename ParentType>
class Forwarder : public Base<Forwarder<ToForward, ParentType>, typename ToForward::ReturnType, ForwarderExtraData<ToForward, ParentType> >
  {
public:
  typedef typename Base<Forwarder<ToForward, ParentType>, typename ToForward::ReturnType, ForwarderExtraData<ToForward, ParentType> >::Iterator Iterator;

  Forwarder(ParentType *p)
      : _parent(p)
    {
    }

  inline void first(Iterator& i) const
    {
    firstInternal(_parent->begin(), i);
    }

  static void next(Iterator &i)
    {
    typename ToForward::Iterator fwd(*i, i.data()._fwdData);
    ToForward::next(fwd);

    if(!*fwd)
      {
      ++(i.data()._parent);
      firstInternal(i.data()._parent, i);
      }
    }

private:
  static void firstInternal(const typename ParentType::Iterator &i, Iterator& ret)
    {
    if(*i)
      {
      ForwarderExtraData<ToForward, ParentType>& d = ret.data();
      d._parent = i;
      d._fwd.reset(*d._parent);

      typename ToForward::Iterator fwdIt;
      d._fwd.first(fwdIt);

      while(!*fwdIt)
        {
        ++(d._parent);
        d._fwd.reset(*d._parent);

        d._fwd.first(fwdIt);
        }

      d._fwdData = fwdIt.data();

      ret.setProperty(*fwdIt);
      }
    else
      {
      ret.setProperty(0);
      }
    }

  ParentType *_parent;
  };

template <typename A, typename B> class Compound : public Forwarder<B, A>
  {
public:
  Compound(SProperty *root) : Forwarder<B, A>(&_a)
    {
    _a.reset(root);
    }

  A _a;
  };

struct ChildTreeExtraData
  {
  SPropertyContainer *_currentParent;
  SProperty *_root;
  };

class ChildTree : public Base<ChildTree, SProperty, ChildTreeExtraData>
  {
public:
  typedef Base<ChildTree, SProperty, ChildTreeExtraData>::Iterator Iterator;

  inline void first(Iterator& it) const
    {
    ChildTreeExtraData &d = it.data();
    d._root = property();
    d._currentParent = d._root->parent();
    it.setProperty(property());
    }

  static void next(Iterator &i)
    {
    SProperty *current = *i;
    SPropertyContainer *cont = current->castTo<SPropertyContainer>();
    if(cont)
      {
      SProperty *child = cont->firstChild();
      if(child)
        {
        ChildTreeExtraData &d = i.data();
        d._currentParent = cont;

        i.setProperty(child);
        return;
        }
      }

    ChildTreeExtraData &d = i.data();
    SProperty *n = d._currentParent->nextSibling(current);

    while(!n && d._currentParent != i.data()._root)
      {
      n = d._currentParent->parent()->nextSibling(d._currentParent);
      d._currentParent = d._currentParent->parent();
      }

    i.setProperty(n);
    }
  };

class ChildEntityTree : public Base<ChildEntityTree, SEntity, ChildTreeExtraData>
  {
public:
  typedef Base<ChildEntityTree, SEntity, ChildTreeExtraData>::Iterator Iterator;

  inline void first(Iterator& i) const
    {
    ChildTreeExtraData &d = i.data();
    d._root = property();
    d._currentParent = d._root->parent();
    i.setProperty(property()->entity());
    }

  inline static void next(Iterator &i)
    {
    SProperty *current = *i;
    SEntity *cont = current->castTo<SEntity>();
    // there is a non-entity in children?
    xAssert(cont);
    if(cont)
      {
      SEntity *child = cont->children.firstChild<SEntity>();
      if(child)
        {
        ChildTreeExtraData &d = i.data();
        d._currentParent = cont;

        i.setProperty(child);
        return;
        }

      ChildTreeExtraData &d = i.data();
      SEntity *n = d._currentParent->nextSibling<SEntity>(cont);

      while(!n && d._currentParent != i.data()._root)
        {
        n = d._currentParent->parent()->nextSibling<SEntity>(d._currentParent);
        d._currentParent = d._currentParent->parent()->parent();
        }

      i.setProperty(n);
      }
    }
  };

template <typename T> class OfType : public Base<OfType<T>, T, NilExtraData>
  {
public:
  typedef typename Base<OfType<T>, T, NilExtraData>::Iterator Iterator;

  inline void first(Iterator &i) const
    {
    SProperty *prop = Base<OfType<T>, T, NilExtraData>::property();
    i.setProperty(prop->castTo<T>());
    }

  inline static void next(Iterator &i)
    {
    i.setProperty(0);
    }
  };
}

#endif // SITERATOR_H
