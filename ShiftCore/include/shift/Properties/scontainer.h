#ifndef SPROPERTYCONTAINER_H
#define SPROPERTYCONTAINER_H

#include "shift/sglobal.h"
#include "shift/Properties/sproperty.h"
#include "shift/Changes/schange.h"

namespace Shift
{

class Container;
class PropertyInstanceInformationInitialiser;

#define S_PROPERTY_CONTAINER S_PROPERTY
#define S_ABSTRACT_PROPERTY_CONTAINER S_ABSTRACT_PROPERTY

#define S_COMPUTE_GROUP(name) static Property PropertyContainer::* name[] = {
#define S_AFFECTS(property) reinterpret_cast<Property PropertyContainer::*>(&className :: property),
#define S_COMPUTE_GROUP_END() 0 };

namespace detail
{
class PropertyContainerTraits;
}

template <typename T, typename Cont> class ContainerBaseIterator;
template <typename T, typename Cont> class ContainerIterator;
template <typename T, typename Cont, typename Iterator> class ContainerTypedIteratorWrapperFrom;

class SHIFT_EXPORT Container : public Property
  {
  S_PROPERTY_CONTAINER(Container, Property, 0);

public:
  typedef detail::PropertyContainerTraits Traits;

  class TreeChange : public Change
    {
    S_CHANGE(TreeChange, Change, Change::TreeChange)
  public:
    TreeChange(Container *b, Container *a, Attribute *ent, xsize index);
    ~TreeChange();
    Container *before(bool back=false)
      {
      if(back)
        {
        return _after;
        }
      return _before;
      }
    const Container *before(bool back=false) const
      {
      if(back)
        {
        return _after;
        }
      return _before;
      }
    Container *after(bool back=false)
      {
      if(back)
        {
        return _before;
        }
      return _after;
      }
    const Container *after(bool back=false) const
      {
      if(back)
        {
        return _before;
        }
      return _after;
      }

    Attribute *property() const {return _attribute;}
    xsize index() const { return _index; }

  private:
    Container *_before;
    Container *_after;
    Attribute *_attribute;
    xsize _index;
    bool _owner;
    bool apply();
    bool unApply();
    bool inform(bool back);
    };

  Container();
  ~Container();

  template <typename T> const T *firstDynamicChild() const
    {
    return ((Container*)this)->firstDynamicChild<T>();
    }

  template <typename T> T *firstDynamicChild()
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

  Attribute *firstChild();
  const Attribute *firstChild() const;
  Attribute *lastChild();
  const Attribute *lastChild() const;

  void disconnectTree();

  Attribute *firstDynamicChild() { preGet(); return _dynamicChild; }
  const Attribute *firstDynamicChild() const { preGet(); return _dynamicChild; }

  template <typename T> const T *nextDynamicSibling(const T *old) const
    {
    return ((Container*)this)->nextDynamicSibling<T>((T*)old);
    }

  template <typename T> T *nextDynamicSibling(const T *old)
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

  Attribute *nextDynamicSibling(const Attribute *p);
  const Attribute *nextDynamicSibling(const Attribute *p) const;

  template <typename T> const T *findChild(const NameArg &name) const
    {
    const Attribute *prop = findChild(name);
    if(prop)
      {
      return prop->castTo<T>();
      }
    return 0;
    }

  template <typename T> T *findChild(const NameArg &name)
    {
    Attribute *prop = findChild(name);
    if(prop)
      {
      return prop->castTo<T>();
      }
    return 0;
    }

  const Attribute *findChild(const NameArg &name) const;
  Attribute *findChild(const NameArg &name);

  bool isEmpty() const { return size() != 0; }

  xsize size() const;
  xuint8 containedProperties() const;

  Attribute *at(xsize i);
  const Attribute *at(xsize i) const;

  // move a property from this to newParent
  void moveAttribute(Container *newParent, Attribute *property);

  bool contains(const Attribute *) const;

  xsize index(const Attribute* prop) const;

  // iterator members, can be used like for (auto prop : container->walker())
  template <typename T> ContainerTypedIteratorWrapperFrom<T, Container, ContainerIterator<T, Container> > walker();
  template <typename T> ContainerTypedIteratorWrapperFrom<const T, const Container, ContainerIterator<const T, const Container> > walker() const;

  template <typename T> ContainerTypedIteratorWrapperFrom<T, Container, ContainerIterator<T, Container> > walkerFrom(T *);
  template <typename T> ContainerTypedIteratorWrapperFrom<const T, const Container, ContainerIterator<const T, const Container> > walkerFrom(const T *) const;

  template <typename T> ContainerTypedIteratorWrapperFrom<T, Container, ContainerIterator<T, Container> > walkerFrom(Attribute *);
  template <typename T> ContainerTypedIteratorWrapperFrom<const T, const Container, ContainerIterator<const T, const Container> > walkerFrom(const Attribute *) const;

  ContainerTypedIteratorWrapperFrom<Attribute, Container, ContainerBaseIterator<Attribute, Container> > walker();
  ContainerTypedIteratorWrapperFrom<const Attribute, const Container, ContainerBaseIterator<const Attribute, const Container> > walker() const;
  ContainerTypedIteratorWrapperFrom<Attribute, Container, ContainerBaseIterator<Attribute, Container> > walkerFrom(Attribute *prop);

protected:
  // contained implies the property is aggregated by the inheriting class and should not be deleted.
  // you cannot add another contained property once dynamic properties have been added, this bool
  // should really be left alone and not exposed in sub classes
  Attribute *addAttribute(const PropertyInformation *info, xsize index=X_SIZE_SENTINEL, const NameArg& name=NameArg(), PropertyInstanceInformationInitialiser *inst=0);
  void removeAttribute(Attribute *);

  void clear();

  // remove and destroy all children. not for use by "array types", use clear instead.
  void internalClear(Database *db);

private:
  Attribute *internalFindChild(const NameArg &name);
  const Attribute *internalFindChild(const NameArg &name) const;
  friend void setDependantsDirty(Property* prop, bool force);

  Attribute *_dynamicChild;

  void terminateTree();

  void makeUniqueName(const Attribute *prop, const NameArg &name, Name &out) const;
  void internalInsert(Attribute *, xsize index);
  void internalSetup(Attribute *);
  void internalRemove(Attribute *);
  void internalUnsetup(Attribute *);

  friend class TreeChange;
  friend class Entity;
  friend class Attribute;
  friend class Database;
  friend class detail::PropertyBaseTraits;
  friend class detail::PropertyContainerTraits;
  };

}

S_PROPERTY_INTERFACE(Shift::Container)

#endif // SPROPERTYCONTAINER_H
