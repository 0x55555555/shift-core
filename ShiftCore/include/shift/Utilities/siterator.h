#ifndef SITERATOR_H
#define SITERATOR_H

#include "shift/sentity.h"
#include "shift/Properties/spropertycontaineriterators.h"

namespace Shift
{

namespace Iterator
{

template <typename IteratorType, typename ReturnTypeIn, typename DerivedExtraData> class Base
  {
public:
  typedef DerivedExtraData ExtraData;
  typedef ReturnTypeIn ReturnType;

  inline Base() : _prop(0)
    {
    }

  inline void reset(Property *prop)
    {
    _prop = prop;
    }

  inline Property *property() const
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

    inline Iterator& operator++()
      {
      IteratorType::next(*this);
      return *this;
      }

    inline Iterator& operator++(int)
      {
        IteratorType::next(*this);
        return *this;
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
  Property *_prop;
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

      while(!*fwdIt && *(++d._parent))
        {
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
  Compound(Property *root) : Forwarder<B, A>(&_a)
    {
    _a.reset(root);
    }

  A _a;
  };

template <typename A, typename B, typename C> class Compound3 : public Forwarder<C, Forwarder<B, A>>
  {
public:
  Compound3(Property *root) : Forwarder<C, Forwarder<B, A>>(&_f), _f(&_a)
    {
    _a.reset(root);
    }

  Forwarder<B, A> _f;
  A _a;
  };

struct ChildTreeExtraData
  {
  PropertyContainer *_currentParent;
  Property *_root;
  };

class ChildTree : public Base<ChildTree, Property, ChildTreeExtraData>
  {
public:
  typedef Base<ChildTree, Property, ChildTreeExtraData>::Iterator Iterator;

  inline void first(Iterator& it) const
    {
    ChildTreeExtraData &d = it.data();
    d._root = property();
    d._currentParent = d._root->parent();
    it.setProperty(property());
    }

  static void next(Iterator &i)
    {
    Property *current = *i;
    PropertyContainer *cont = current->castTo<PropertyContainer>();
    if(cont)
      {
      Property *child = cont->firstChild();
      if(child)
        {
        ChildTreeExtraData &d = i.data();
        d._currentParent = cont;

        i.setProperty(child);
        return;
        }
      }

    ChildTreeExtraData &d = i.data();
    auto walker = d._currentParent->walkerFrom(current);
    auto walkerIt = ++walker.begin();
    Property *n = *walkerIt;

    while(!n && d._currentParent != i.data()._root)
      {
      PropertyContainer *parent = d._currentParent->parent();

      auto walker = parent->walkerFrom(d._currentParent);
      auto walkerIt = ++walker.begin();
      n = *walkerIt;

      d._currentParent = parent;
      }

    i.setProperty(n);
    }
  };

class ChildEntityTree : public Base<ChildEntityTree, Entity, ChildTreeExtraData>
  {
public:
  typedef Base<ChildEntityTree, Entity, ChildTreeExtraData>::Iterator Iterator;

  inline void first(Iterator& i) const
    {
    if(!property())
      {
      return;
      }
    ChildTreeExtraData &d = i.data();
    d._currentParent = property()->parent();
    d._root = d._currentParent;
    xAssert(property()->entity() == property());
    i.setProperty(property()->entity());
    }

  inline static void next(Iterator &i)
    {
    Entity *current = *i;
    // there is a non-entity in children?
    xAssert(current);
    if(current)
      {
      Entity *child = *current->children.walker<Entity>().begin();
      if(child)
        {
        ChildTreeExtraData &d = i.data();
        d._currentParent = &current->children;

        i.setProperty(child);
        return;
        }

      ChildTreeExtraData &d = i.data();
      auto walker = d._currentParent->walkerFrom<Entity>(current);
      auto walkerIt = ++walker.begin();
      Entity *n = *walkerIt;

      while(!n && d._currentParent != i.data()._root)
        {
        // get the parent's (children member) parent (should be an entity,
        // in another children member) and get its next sibling.
        Entity* parentEntity = d._currentParent->parent()->castTo<Entity>();
        PropertyContainer *parent = parentEntity->parent();

        auto walker = parent->walkerFrom<Entity>(parentEntity);
        auto walkerIt = ++walker.begin();
        n = *walkerIt;
        d._currentParent = parent;
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
    Property *prop = Base<OfType<T>, T, NilExtraData>::property();
    i.setProperty(prop->castTo<T>());
    }

  inline static void next(Iterator &i)
    {
    i.setProperty(0);
    }
  };

}

}

#endif // SITERATOR_H