#include "shift/Utilities/satomichelper.h"
#include "Memory/XResourceDescription.h"
#include "Utilities/XAssert.h"
#include "QtGlobal"

#if defined(X_MSVC)
# include "Windows.h"
#endif

namespace Shift
{

namespace AtomicHelper
{

bool trySet(Type *p, Type newVal, Type oldVal)
  {
  bool success = false;

#if defined(X_MSVC)
  xAssertIsSpecificAligned(p, 8);
  // try to lock the updating flag, if we cant, someone else is working.
  success = InterlockedCompareExchange16(p, newVal, oldVal) == oldVal;

#elif defined(Q_CC_GCC)
  success = __sync_val_compare_and_swap(p, oldVal, newVal) == oldVal;
#endif

  return success;
  }

}

}
