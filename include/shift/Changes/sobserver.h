#ifndef SOBSERVER_H
#define SOBSERVER_H

#include "Containers/XVector.h"
#include "shift/sglobal.h"
#include "shift/Properties/sproperty.h"

namespace Shift
{

class Change;

class SHIFT_EXPORT Observer
  {
public:
  Observer();
  virtual ~Observer();
  virtual void actOnChanges() { };

private:
  X_DISABLE_COPY(Observer);
  Eks::Vector <Entity*> _entities;
  friend class Entity;
  };

class SHIFT_EXPORT TreeObserver : public virtual Observer
  {
public:
  virtual ~TreeObserver() { }
  virtual void onTreeChange(const Change *, bool backwards) = 0;
  };

class SHIFT_EXPORT ConnectionObserver : public virtual Observer
  {
public:
  virtual ~ConnectionObserver() { }
  virtual void onConnectionChange(const Change *, bool backwards) = 0;
  };

}

#endif // SOBSERVER_H
