#include "shift/Serialisation/sioblock.h"

namespace Shift
{

IOBlock::IOBlock(IOBlockUser* w)
  : _user(w),
    _active(false),
    _complete(false)
  {
  xAssert(!w->_block);
  w->_block = this;
  }

IOBlock::~IOBlock()
  {
  _user->_block = nullptr;
  }

IOBlockUser::IOBlockUser()
  : _block(nullptr)
  {
  }

IOBlockUser::~IOBlockUser()
  {
  xAssert(!_block);
  }

void IOBlockUser::begin()
  {
  xAssert(_block);
  xAssert(!_block->_active);
  xAssert(!_block->_complete);

  _block->_active = true;
  }

void IOBlockUser::end()
  {
  xAssert(_block);
  xAssert(_block->_active);
  xAssert(!_block->_complete);

  _block->_active = false;
  _block->_complete = true;
  }
}
