#ifndef SHIFTTESTCOMMON_H
#define SHIFTTESTCOMMON_H

#include "shift/sdatabase.h"

#define S_IMPLEMENT_TEST \
namespace Test { \
Shift::PropertyGroup &propertyGroup() { \
  static Shift::PropertyGroup grp; \
  return grp; } }

namespace Test
{
Shift::PropertyGroup &propertyGroup();
}

class TestDatabase : public Shift::Database
  {
public:
  TestDatabase()
    {
    initiateInheritedDatabaseType(staticTypeInformation());
    }
  };

namespace QTest
{

template <typename T1, typename T2>
inline bool qCompare(T1 const &t1, T2 const &t2, const char *actual, const char *expected,
                    const char *file, int line)
{
    return compare_helper(t1 == t2, "Compared values are not the same",
                          toString<T1>(t1), toString<T2>(t2), actual, expected, file, line);
}

}

#endif // SHIFTTESTCOMMON_H
