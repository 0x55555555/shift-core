#include "shift/Changes/schange.h"

namespace Shift
{

namespace detail
{
xuint32 MetaType::newId()
  {
  static xuint32 id = 0;
  ++id;
  return id;
  }
}

}
