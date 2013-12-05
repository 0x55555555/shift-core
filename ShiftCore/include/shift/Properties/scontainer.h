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
  S_PROPERTY_CONTAINER(Container, Property);

public:
  typedef detail::PropertyContainerTraits Traits;

  Container();
  ~Container();

  class EditCache;
  Eks::UniquePointer<EditCache> createEditCache(Eks::AllocatorBase *alloc);

  template <typename T> inline const T *firstDynamicChild() const;
  template <typename T> inline T *firstDynamicChild();

  Attribute *firstChild();
  const Attribute *firstChild() const;
  Attribute *lastChild();
  const Attribute *lastChild() const;

  void disconnectTree();

  inline Attribute *firstDynamicChild();
  inline const Attribute *firstDynamicChild() const;

  inline Attribute *lastDynamicChild();
  inline const Attribute *lastDynamicChild() const;

  template <typename T> inline const T *nextDynamicSibling(const T *old) const;
  template <typename T> inline T *nextDynamicSibling(const T *old);

  Attribute *nextDynamicSibling(const Attribute *p);
  const Attribute *nextDynamicSibling(const Attribute *p) const;

  template <typename T> inline T *findChild(const NameArg &name);
  template <typename T> inline const T *findChild(const NameArg &name) const;

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

  template <typename T> ContainerTypedIteratorWrapperFrom<T, Container, ContainerIterator<T, Container> > walkerFromTyped(T *);
  template <typename T> ContainerTypedIteratorWrapperFrom<const T, const Container, ContainerIterator<const T, const Container> > walkerFromTyped(const T *) const;

  template <typename T> ContainerTypedIteratorWrapperFrom<T, Container, ContainerIterator<T, Container> > walkerFromTyped(Attribute *);
  template <typename T> ContainerTypedIteratorWrapperFrom<const T, const Container, ContainerIterator<const T, const Container> > walkerFromTyped(const Attribute *) const;

  typedef ContainerTypedIteratorWrapperFrom<Attribute, Container, ContainerBaseIterator<Attribute, Container> > Walker;
  typedef ContainerTypedIteratorWrapperFrom<const Attribute, const Container, ContainerBaseIterator<const Attribute, const Container> > ConstWalker;

  Walker walker();
  ConstWalker walker() const;
  Walker walkerFrom(Attribute *prop);

  void assignUniqueName(Attribute *p, const NameArg &hint) const;

protected:
  // contained implies the property is aggregated by the inheriting class and should not be deleted.
  // you cannot add another contained property once dynamic properties have been added, this bool
  // should really be left alone and not exposed in sub classes
  Attribute *addAttribute(
    const PropertyInformation *info,
    xsize index=X_SIZE_SENTINEL,
    const NameArg& name=NameArg(),
    PropertyInstanceInformationInitialiser *inst=0);
  void removeAttribute(Attribute *);

  void clear();

  // remove and destroy all children. not for use by "array types", use clear instead.
  void internalClear(Database *db);

private:
  Attribute *internalFindChild(const NameArg &name);
  const Attribute *internalFindChild(const NameArg &name) const;
  friend void setDependantsDirty(Property* prop, bool force);

  Attribute *_dynamicChild;
  Attribute *_lastDynamicChild;

  bool makeUniqueName(const Attribute *prop, const NameArg &name, Name &out) const;
  void internalInsert(Attribute *, xsize index);
  void internalSetup(Attribute *);
  void internalRemove(Attribute *);
  void internalUnsetup(Attribute *);

  friend class ContainerTreeChange;
  friend class Entity;
  friend class Attribute;
  friend class Database;
  friend class detail::PropertyBaseTraits;
  friend class detail::PropertyContainerTraits;
  };

}

S_PROPERTY_INTERFACE(Shift::Container)

#endif // SPROPERTYCONTAINER_H
