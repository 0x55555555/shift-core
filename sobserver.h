#ifndef SOBSERVER_H
#define SOBSERVER_H

#include "sglobal.h"
#include "XVector"
#include "sproperty.h"

class SChange;

class SHIFT_EXPORT SObserver
  {
public:
  SObserver() { }
  virtual ~SObserver();
  virtual void actOnChanges() { };

private:
  X_DISABLE_COPY(SObserver);
  QVector <SEntity*> _entities;
  friend class SEntity;
  };

class SHIFT_EXPORT STreeObserver : public virtual SObserver
  {
public:
  virtual ~STreeObserver() { }
  virtual void onTreeChange(const SChange *, bool backwards) = 0;
  };

X_SCRIPTABLE_TYPE(STreeObserver);

class SHIFT_EXPORT SDirtyObserver : public virtual SObserver
  {
public:
  virtual ~SDirtyObserver() { }
  virtual void onPropertyDirtied(const SProperty*) = 0;
  };

X_SCRIPTABLE_TYPE(SDirtyObserver);

class SHIFT_EXPORT SConnectionObserver : public virtual SObserver
  {
public:
  virtual ~SConnectionObserver() { }
  virtual void onConnectionChange(const SChange *, bool backwards) = 0;
  };

X_SCRIPTABLE_TYPE(SConnectionObserver);

#endif // SOBSERVER_H
