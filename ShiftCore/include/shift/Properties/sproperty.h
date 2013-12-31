#ifndef SPROPERTY_H
#define SPROPERTY_H

#include "shift/sglobal.h"
#include "shift/Properties/sattribute.h"
#include "shift/Utilities/satomichelper.h"
#include "Utilities/XFlags.h"

namespace Shift
{

class Entity;
class PropertyMetaData;
class PropertyConnectionChange;
class SResourceDescription;

class SHIFT_EXPORT Property : public Attribute
  {
public:
  typedef PropertyInstanceInformation BaseInstanceInformation;
  typedef EmbeddedPropertyInstanceInformation EmbeddedInstanceInformation;
  typedef DynamicPropertyInstanceInformation DynamicInstanceInformation;
  typedef detail::PropertyBaseTraits Traits;

  S_PROPERTY(Property, Attribute)

public:
  Property();

  Property *input() const { return _input; }
  Property *output() const { return _output; }
  Property *nextOutput() const { return _nextOutput; }

  template <typename T> T *output() const;

  // connect this property (driver) to the passed property (driven)
  void connect(Property *) const;
  void setInput(const Property *inp);
  void connect(const Eks::Vector<Property*> &) const;
  void disconnect(Property *) const;
  void disconnect() const;

  bool outputsTo(const Property *p) const;

  bool isDirty() const { return _dirty; }
  bool isUpdating() const { return _updating; }
  bool isComputed() const;
  bool hasInput() const { return _input; }
  bool hasOutputs() const { return _output; }
  bool parentHasInput() const;
  bool parentHasOutput() const;
  Eks::Vector<const Property *> affects() const;
  Eks::Vector<Property *> affects();

  void postSet();
  void postCompute();
  void setDependantsDirty();
  void preGet() const
    {
    if(isDirty())
      {
      update();
      }
    }
  void concurrentPreGet() const;


  typedef PropertyConnectionChange ConnectionChange;

private:
  bool beginUpdate() const;
  void endUpdate() const;

  void concurrentUpdate() const;
  void update() const;
  void updateParent() const;

  void setDirty();

  void connectInternal(Property *) const;
  void disconnectInternal(Property *) const;

  Property *_input;
  Property *_output;
  Property *_nextOutput;

  AtomicHelper::Type _updating;
  enum Flags
    {
    ParentHasInput = 1,
    ParentHasOutput = 2
    };
  Eks::Flags<Flags, xuint8> _flags;
  xuint8 _dirty;

  friend class PropertyConnectionChange;
  friend class NoUpdateBlock;
  friend class Container;
  friend class Database;
  };

template <typename T> inline T *Property::output() const
  {
  Property *p = output();
  while(p)
    {
    T *t = p->castTo<T>();
    if(t)
      {
      return t;
      }
    p = p->nextOutput();
    }
  return 0;
  }

class NoUpdateBlock
  {
public:
  NoUpdateBlock(Attribute *);
  ~NoUpdateBlock();

private:
  Property *_prop;
  bool _oldDirty;
  };
}

S_PROPERTY_INTERFACE(Shift::Property)

#endif // SPROPERTY_H
