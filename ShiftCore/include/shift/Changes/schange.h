#ifndef SCHANGE_H
#define SCHANGE_H

#include "shift/sglobal.h"
#include "XProperty"
#include "XObject"

#define S_CHANGE_ROOT(id) public: enum {Type = (id)}; virtual xuint32 type() const { return id; } X_CASTABLE_ROOT( Change, id, static_cast<T*>(0)->Type )

#define S_CHANGE(cl, supCl, id) public: enum {Type = (id)}; virtual xuint32 type() const { return id; } X_CASTABLE( cl, supCl, id, Shift::Change )

namespace Shift
{

class Change
  {
  S_CHANGE_ROOT(1);
public:
  virtual ~Change() { }
  virtual bool apply() = 0;
  virtual bool unApply() = 0;
  virtual bool inform(bool backwards) = 0;
  };

}

#endif // SCHANGE_H
