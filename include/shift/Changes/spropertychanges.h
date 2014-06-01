#ifndef SPROPERTYCHANGES_H
#define SPROPERTYCHANGES_H

#include "shift/Changes/schange.h"
#include "shift/Utilities/spropertyname.h"

namespace Shift
{

class Container;

class PropertyDataChange : public Change
  {
  S_CHANGE(PropertyDataChange, Change, Change::BaseDataChange)
public:
  PropertyDataChange(Property *p) : _property(p) { }
  Property *property() {return _property;}
  const Property *property() const {return _property;}
private:
  Property *_property;
  };

class PropertyNameChange : public Change
  {
  S_CHANGE(PropertyNameChange, Change, Change::NameChange)

public:
  PropertyNameChange(const NameArg &b, const NameArg &a, Attribute *ent)
    : _attribute(ent)
    {
    b.toName(_before);
    a.toName(_after);
    }
  const Name &before(bool back=false) const
    {
    if(back)
      {
      return _after;
      }
    return _before;
    }
  const Name &after(bool back=false) const
    {
    if(back)
      {
      return _before;
      }
    return _after;
    }
  Attribute *attribute() {return _attribute;}
  const Attribute *attribute() const {return _attribute;}
private:
  Name _before;
  Name _after;
  Attribute *_attribute;
  bool apply();
  bool unApply();
  bool inform(bool back);
  };

class PropertyConnectionChange : public Change
  {
  S_CHANGE(PropertyConnectionChange, Change, Change::ConnectionChange)
public:
  enum Mode
    {
    Connect,
    Disconnect
    };

  PropertyConnectionChange(Mode m, Property *driver, Property *driven)
    : _driver(driver), _driven(driven), _mode(m)
    { }
  Property *driver() { return _driver; }
  Property *driven() { return _driven; }
  const Property *driver() const { return _driver; }
  const Property *driven() const { return _driven; }
  Mode mode(bool back=false) const
    {
    if(back)
      {
      return (Mode)(_mode - Disconnect);
      }
    return _mode;
    }

  static void setParentHasInputConnection(Property *);
  static void setParentHasOutputConnection(Property *);
  static void clearParentHasInputConnection(Property *);
  static void clearParentHasOutputConnection(Property *);

private:
  Property *_driver;
  Property *_driven;
  Mode _mode;
  bool apply();
  bool unApply();
  bool inform(bool back);
  };

class ContainerTreeChange : public Change
  {
  S_CHANGE(ContainerTreeChange, Change, Change::TreeChange)
public:
  ContainerTreeChange(Container *b, Container *a, Attribute *ent, xsize index);
  ~ContainerTreeChange();
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

}

#endif // SPROPERTYCHANGES_H
