#ifndef SPROPERTYCHANGES_H
#define SPROPERTYCHANGES_H

#include "shift/Changes/schange.h"

namespace Shift
{

class PropertyDataChange : public Change
  {
  S_CHANGE(PropertyDataChange, Change, 53)
public:
  PropertyDataChange(Property *p) : _property(p) { }
  Property *property() {return _property;}
  const Property *property() const {return _property;}
private:
  Property *_property;
  };

class PropertyNameChange : public Change
  {
  S_CHANGE(PropertyNameChange, Change, 50)
public:
  PropertyNameChange(const QString &b, const QString &a, Property *ent)
    : _before(b), _after(a), _property(ent)
    { }
  const QString &before(bool back=false) const
    {
    if(back)
      {
      return _after;
      }
    return _before;
    }
  const QString &after(bool back=false) const
    {
    if(back)
      {
      return _before;
      }
    return _after;
    }
  Property *property() {return _property;}
  const Property *property() const {return _property;}
private:
  QString _before;
  QString _after;
  Property *_property;
  bool apply();
  bool unApply();
  bool inform(bool back);
  };

class PropertyConnectionChange : public Change
  {
  S_CHANGE(PropertyConnectionChange, Change, 51)
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

}

#endif // SPROPERTYCHANGES_H
