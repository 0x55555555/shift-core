#ifndef SPROPERTYCONTAINER_H
#define SPROPERTYCONTAINER_H

#include "shift/sglobal.h"
#include "shift/Properties/sproperty.h"
#include "shift/Changes/schange.h"

namespace Shift
{

class PropertyContainer;
class PropertyInstanceInformationInitialiser;

#define S_PROPERTY_CONTAINER S_PROPERTY
#define S_ABSTRACT_PROPERTY_CONTAINER S_ABSTRACT_PROPERTY

#define S_COMPUTE_GROUP(name) static Property PropertyContainer::* name[] = {
#define S_AFFECTS(property) reinterpret_cast<Property PropertyContainer::*>(&className :: property),
#define S_COMPUTE_GROUP_END() 0 };


template <typename T, typename Cont> class PropertyContainerBaseIterator;
template <typename T, typename Cont> class PropertyContainerIterator;
template <typename T, typename Cont, typename Iterator> class PropertyContainerTypedIteratorWrapperFrom;

class SHIFT_EXPORT PropertyContainer : public Property
  {
  S_PROPERTY_CONTAINER(PropertyContainer, Property, 0);

public:
  class TreeChange : public Change
    {
    S_CHANGE(TreeChange, Change, 52)
  public:
    TreeChange(PropertyContainer *b, PropertyContainer *a, Property *ent, xsize index);
    ~TreeChange();
    PropertyContainer *before(bool back=false)
      {
      if(back)
        {
        return _after;
        }
      return _before;
      }
    const PropertyContainer *before(bool back=false) const
      {
      if(back)
        {
        return _after;
        }
      return _before;
      }
    PropertyContainer *after(bool back=false)
      {
      if(back)
        {
        return _before;
        }
      return _after;
      }
    const PropertyContainer *after(bool back=false) const
      {
      if(back)
        {
        return _before;
        }
      return _after;
      }

    Property *property() const {return _property;}
    xsize index() const { return _index; }

  private:
    PropertyContainer *_before;
    PropertyContainer *_after;
    Property *_property;
    xsize _index;
    bool _owner;
    bool apply();
    bool unApply();
    bool inform(bool back);
    };

  PropertyContainer();
  ~PropertyContainer();

  template <typename T> const T *firstDynamicChild() const
    {
    return ((PropertyContainer*)this)->firstDynamicChild<T>();
    }

  template <typename T> T *firstDynamicChild()
    {
    Property *prop = firstDynamicChild();
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

  Property *firstChild();
  const Property *firstChild() const;
  Property *lastChild();
  const Property *lastChild() const;

  void disconnectTree();

  Property *firstDynamicChild() { preGet(); return _dynamicChild; }
  const Property *firstDynamicChild() const { preGet(); return _dynamicChild; }

  template <typename T> const T *nextDynamicSibling(const T *old) const
    {
    return ((PropertyContainer*)this)->nextDynamicSibling<T>((T*)old);
    }

  template <typename T> T *nextDynamicSibling(const T *old)
    {
    Property *prop = nextDynamicSibling((const Property*)old);
    while(prop)
      {
      T *t = prop->castTo<T>();
      if(t)
        {
        return t;
        }
      prop = nextDynamicSibling((const Property*)prop);
      }
    return 0;
    }

  Property *nextDynamicSibling(const Property *p);
  const Property *nextDynamicSibling(const Property *p) const;

  template <typename T> const T *findChild(const QString &name) const
    {
    const Property *prop = findChild(name);
    if(prop)
      {
      return prop->castTo<T>();
      }
    return 0;
    }

  template <typename T> T *findChild(const QString &name)
    {
    Property *prop = findChild(name);
    if(prop)
      {
      return prop->castTo<T>();
      }
    return 0;
    }

  const Property *findChild(const QString &name) const;
  Property *findChild(const QString &name);

  bool isEmpty() const { return _containedProperties != 0; }

  xsize size() const;
  xsize containedProperties() const { return _containedProperties; }

  Property *at(xsize i);
  const Property *at(xsize i) const;

  // move a property from this to newParent
  void moveProperty(PropertyContainer *newParent, Property *property);

  bool contains(const Property *) const;

  xsize index(const Property* prop) const;

  static void assignProperty(const Property *, Property *);
  static void saveProperty(const Property *, Saver & );
  static Property *loadProperty(PropertyContainer *, Loader &);
  static bool shouldSavePropertyValue(const Property *);

  // iterator members, can be used like for (auto prop : container->walker())
  template <typename T> PropertyContainerTypedIteratorWrapperFrom<T, PropertyContainer, PropertyContainerIterator<T, PropertyContainer> > walker();
  template <typename T> PropertyContainerTypedIteratorWrapperFrom<const T, const PropertyContainer, PropertyContainerIterator<const T, const PropertyContainer> > walker() const;

  template <typename T> PropertyContainerTypedIteratorWrapperFrom<T, PropertyContainer, PropertyContainerIterator<T, PropertyContainer> > walkerFrom(T *);
  template <typename T> PropertyContainerTypedIteratorWrapperFrom<const T, const PropertyContainer, PropertyContainerIterator<const T, const PropertyContainer> > walkerFrom(const T *) const;

  template <typename T> PropertyContainerTypedIteratorWrapperFrom<T, PropertyContainer, PropertyContainerIterator<T, PropertyContainer> > walkerFrom(Property *);
  template <typename T> PropertyContainerTypedIteratorWrapperFrom<const T, const PropertyContainer, PropertyContainerIterator<const T, const PropertyContainer> > walkerFrom(const Property *) const;

  PropertyContainerTypedIteratorWrapperFrom<Property, PropertyContainer, PropertyContainerBaseIterator<Property, PropertyContainer> > walker();
  PropertyContainerTypedIteratorWrapperFrom<const Property, const PropertyContainer, PropertyContainerBaseIterator<const Property, const PropertyContainer> > walker() const;
  PropertyContainerTypedIteratorWrapperFrom<Property, PropertyContainer, PropertyContainerBaseIterator<Property, PropertyContainer> > walkerFrom(Property *prop);

  X_ALIGNED_OPERATOR_NEW

protected:
  // contained implies the property is aggregated by the inheriting class and should not be deleted.
  // you cannot add another contained property once dynamic properties have been added, this bool
  // should really be left alone and not exposed in sub classes
  Property *addProperty(const PropertyInformation *info, xsize index=X_SIZE_SENTINEL, const QString& name=QString(), PropertyInstanceInformationInitialiser *inst=0);
  void removeProperty(Property *);

  void clear();

  // remove and destroy all children. not for use by "array types", use clear instead.
  void internalClear(Database *db);

private:
  Property *internalFindChild(const QString &name);
  const Property *internalFindChild(const QString &name) const;
  friend void setDependantsDirty(Property* prop, bool force);
  Property *_dynamicChild;
  xsize _containedProperties;
  Database *_database;

  QString makeUniqueName(const QString &name) const;
  void internalInsertProperty(Property *, xsize index);
  void internalSetupProperty(Property *);
  void internalRemoveProperty(Property *);
  void internalUnsetupProperty(Property *);

  friend class TreeChange;
  friend class Entity;
  friend class Property;
  friend class Database;
  };

}

S_PROPERTY_INTERFACE(Shift::PropertyContainer)

#endif // SPROPERTYCONTAINER_H
