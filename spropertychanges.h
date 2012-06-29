#ifndef SPROPERTYCHANGES_H
#define SPROPERTYCHANGES_H

class SPropertyDataChange : public SChange
  {
  S_CHANGE(SPropertyDataChange, SChange, 53)
public:
  SPropertyDataChange(SProperty *p) : _property(p) { }
  SProperty *property() {return _property;}
  const SProperty *property() const {return _property;}
private:
  SProperty *_property;
  };

class SPropertyNameChange : public SChange
  {
  S_CHANGE(SPropertyNameChange, SChange, 50)
public:
  SPropertyNameChange(const QString &b, const QString &a, SProperty *ent)
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
  SProperty *property() {return _property;}
  const SProperty *property() const {return _property;}
private:
  QString _before;
  QString _after;
  SProperty *_property;
  bool apply();
  bool unApply();
  bool inform(bool back);
  };

class SPropertyConnectionChange : public SChange
  {
  S_CHANGE(SPropertyConnectionChange, SChange, 51)
public:
  enum Mode
    {
    Connect,
    Disconnect
    };

  SPropertyConnectionChange(Mode m, SProperty *driver, SProperty *driven)
    : _driver(driver), _driven(driven), _mode(m)
    { }
  SProperty *driver() { return _driver; }
  SProperty *driven() { return _driven; }
  const SProperty *driver() const { return _driver; }
  const SProperty *driven() const { return _driven; }
  Mode mode(bool back=false) const
    {
    if(back)
      {
      return (Mode)(_mode - Disconnect);
      }
    return _mode;
    }

  static void setParentHasInputConnection(SProperty *);
  static void setParentHasOutputConnection(SProperty *);
  static void clearParentHasInputConnection(SProperty *);
  static void clearParentHasOutputConnection(SProperty *);

private:
  SProperty *_driver;
  SProperty *_driven;
  Mode _mode;
  bool apply();
  bool unApply();
  bool inform(bool back);
  };

#endif // SPROPERTYCHANGES_H
