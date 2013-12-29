#ifndef SIOBLOCK_H
#define SIOBLOCK_H

#include "Utilities/XProperty.h"
#include "Utilities/XAssert.h"
#include "shift/sglobal.h"

namespace Shift
{

class IOBlockUser;

class SHIFT_EXPORT IOBlock
  {
XProperties:
  XROProperty(IOBlockUser *, user);
  XROProperty(bool, active);
  XROProperty(bool, complete);

protected:
  IOBlock(IOBlockUser *user);
  ~IOBlock();

  friend class IOBlockUser;
  };

class IOBlockUser
  {
protected:
  IOBlockUser();
  ~IOBlockUser();

  /// \brief Get the active IO block.
  IOBlock *activeBlock() { xAssert(_block); return _block; }

  void begin();
  void end();

private:
  IOBlock *_block;

  friend class IOBlock;
  };

}

#endif // SIOBLOCK_H
