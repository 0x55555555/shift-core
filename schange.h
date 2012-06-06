#ifndef SCHANGE_H
#define SCHANGE_H

#include "XProperty"
#include "sglobal.h"
#include "XObject"

#define S_CHANGE_ROOT(id) public: enum {Type = (id)}; virtual xuint32 type() const { return id; } X_CASTABLE_ROOT( SChange, id, static_cast<T*>(0)->Type )

#define S_CHANGE(cl, supCl, id) public: enum {Type = (id)}; virtual xuint32 type() const { return id; } X_CASTABLE( cl, supCl, id, SChange )

class SChange
  {
  S_CHANGE_ROOT(1);
public:
  virtual ~SChange() { }
  virtual bool apply() = 0;
  virtual bool unApply() = 0;
  virtual bool inform(bool backwards) = 0;
  };

#endif // SCHANGE_H
