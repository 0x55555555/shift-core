#ifndef METATYPE_H
#define METATYPE_H

#include "shift/sglobal.h"
#include "Containers/XStringSimple.h"

namespace Shift
{

namespace detail
{


class SHIFT_EXPORT MetaType
  {
public:
  template <typename T> static xuint32 id()
    {
    static xuint32 id = newId();
    return id;
    }

private:
  static xuint32 newId();
  };

template <typename T> class MetaTypeHelper
  {
public:
  static inline void getTypeName(Eks::String &str)
    {
    xAssertFail();

    str.appendType(MetaType::id<T>());
    }
  };

}

}

#define S_DECLARE_METATYPE(TYPE, NAME) \
template <> class Shift::detail::MetaTypeHelper<TYPE> { public: \
  static void getTypeName(Eks::String &str) { \
    str.append(#NAME); } };

#endif // METATYPE_H
