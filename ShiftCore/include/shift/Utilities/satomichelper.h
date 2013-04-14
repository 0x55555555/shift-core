#ifndef SATOMICHELPER_H
#define SATOMICHELPER_H

#include "shift/sglobal.h"

namespace Shift
{

namespace AtomicHelper
{

#ifdef Q_CC_MSVC
typedef volatile short Type;
#else
typedef xuint16 Type;
#endif

SHIFT_EXPORT bool trySet(Type *p, Type newVal, Type oldVal);

}
}

#endif // SATOMICHELPER_H
