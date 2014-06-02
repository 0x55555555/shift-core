#include "shift/Changes/schange.h"

namespace Shift
{

Change::~Change()
  {
  }

Change *Change::castToType(xuint32 id)
  {
  return (Change*)((const Change*)this)->castToType(id);
  }

const Change *Change::castToType(xuint32 id) const
  {
  if(id == Type)
    {
    return this;
    }

  return nullptr;
  }

}
