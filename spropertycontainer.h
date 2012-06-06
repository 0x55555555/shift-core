#ifndef SPROPERTYCONTAINER_H
#define SPROPERTYCONTAINER_H

#include "sglobal.h"
#include "sproperty.h"
#include "schange.h"

class SPropertyContainer;
class SPropertyInstanceInformationInitialiser;

#define S_PROPERTY_CONTAINER S_PROPERTY
#define S_ABSTRACT_PROPERTY_CONTAINER S_ABSTRACT_PROPERTY

#define S_COMPUTE_GROUP(name) static SProperty SPropertyContainer::* name[] = {
#define S_AFFECTS(property) reinterpret_cast<SProperty SPropertyContainer::*>(&className :: property),
#define S_COMPUTE_GROUP_END() 0 };

class SHIFT_EXPORT SPropertyContainer : public SProperty
  {
  S_PROPERTY_CONTAINER(SPropertyContainer, SProperty, 0);

public:
  class TreeChange : public SChange
    {
    S_CHANGE(TreeChange, SChange, 52)
  public:
    TreeChange(SPropertyContainer *b, SPropertyContainer *a, SProperty *ent, xsize index);
    ~TreeChange();
    SPropertyContainer *before(bool back=false)
      {
      if(back)
        {
        return _after;
        }
      return _before;
      }
    const SPropertyContainer *before(bool back=false) const
      {
      if(back)
        {
        return _after;
        }
      return _before;
      }
    SPropertyContainer *after(bool back=false)
      {
      if(back)
        {
        return _before;
        }
      return _after;
      }
    const SPropertyContainer *after(bool back=false) const
      {
      if(back)
        {
        return _before;
        }
      return _after;
      }

    SProperty *property() const {return _property;}
    xsize index() const { return _index; }
  private:
    SPropertyContainer *_before;
    SPropertyContainer *_after;
    SProperty *_property;
    xsize _index;
    bool _owner;
    bool apply();
    bool unApply();
    bool inform(bool back);
    };

  SPropertyContainer();
  ~SPropertyContainer();

  template <typename T> T *firstChild() const
    {
    SProperty *prop = firstChild();
    while(prop)
      {
      T *t = prop->castTo<T>();
      if(t)
        {
        return t;
        }
      prop = prop->nextSibling();
      }
    return 0;
    }

  SProperty *firstChild() const { preGet(); return _child; }

  template <typename T> const T *findChild(const QString &name) const
    {
    const SProperty *prop = findChild(name);
    if(prop)
      {
      return prop->castTo<T>();
      }
    return 0;
    }

  template <typename T> T *findChild(const QString &name)
    {
    SProperty *prop = findChild(name);
    if(prop)
      {
      return prop->castTo<T>();
      }
    return 0;
    }

  const SProperty *findChild(const QString &name) const;
  SProperty *findChild(const QString &name);

  xsize size() const;
  xsize containedProperties() const { return _containedProperties; }

  SProperty *at(xsize i);
  const SProperty *at(xsize i) const;

  // move a property from this to newParent
  void moveProperty(SPropertyContainer *newParent, SProperty *property);

  bool contains(SProperty *) const;

  static void assignProperty(const SProperty *, SProperty *);
  static void saveProperty(const SProperty *, SSaver & );
  static SProperty *loadProperty(SPropertyContainer *, SLoader &);
  static bool shouldSavePropertyValue(const SProperty *);

  static void postChildSet(SPropertyContainer *, SProperty *);

  X_ALIGNED_OPERATOR_NEW

protected:
  // contained implies the property is aggregated by the inheriting class and should not be deleted.
  // you cannot add another contained property once dynamic properties have been added, this bool
  // should really be left alone and not exposed in sub classes
  SProperty *addProperty(const SPropertyInformation *info, xsize index=X_SIZE_SENTINEL, const QString& name=QString(), SPropertyInstanceInformationInitialiser *inst=0);
  void removeProperty(SProperty *);

  void clear();

  // remove and destroy all children. not for use by "array types", use clear instead.
  void internalClear();

private:
  SProperty *internalFindChild(const QString &name);
  const SProperty *internalFindChild(const QString &name) const;
  friend void setDependantsDirty(SProperty* prop, bool force);
  SProperty *_child;
  xsize _containedProperties;

  QString makeUniqueName(const QString &name) const;
  void internalInsertProperty(bool contained, SProperty *, xsize index);
  void internalRemoveProperty(SProperty *);

  friend class TreeChange;
  friend class SEntity;
  friend class SProperty;
  friend class SDatabase;
  };

S_PROPERTY_INTERFACE(SPropertyContainer)

#endif // SPROPERTYCONTAINER_H
