#ifndef COMPUTETEST_H
#define COMPUTETEST_H

#include <QtTest>
#include "XCore"
#include "XGlobalAllocator"
#include "shift/sentity.h"
#include "shift/sdatabase.h"
#include "shift/Properties/sbaseproperties.h"

// future tests
// casting test, benchmarking shallow and deep hierarchies
// iteration test, and benchmarks, including typing...

namespace Test
{
Shift::PropertyGroup &propertyGroup();
}

class ShiftCoreTest : public QObject
  {
  Q_OBJECT

public:
  ShiftCoreTest()
    {
    Shift::TypeRegistry::initiate(core.defaultAllocator());

    Shift::TypeRegistry::addPropertyGroup(Test::propertyGroup());
    }

  ~ShiftCoreTest()
    {
    Shift::TypeRegistry::terminate();
    }

private Q_SLOTS:
  void simpleOperationTest();
  void simpleDirtyCompute();

private:
  Eks::Core core;
  };

class TestVector : public Shift::Container
  {
  S_ENTITY(TestVector, Container, 0)
public:

  Shift::FloatProperty x;
  Shift::FloatProperty y;
  Shift::FloatProperty z;

  bool allDirty() const
    {
    return countDirty() == 4;
    }

  bool anyDirty() const
    {
    return countDirty() != 0;
    }

  xsize countDirty() const
    {
    xsize count = 0;
    isDirty() ? ++count : count;
    x.isDirty() ? ++count : count;
    y.isDirty() ? ++count : count;
    z.isDirty() ? ++count : count;

    return count;
    }
  };

class TestEntity : public Shift::Entity
  {
  S_ENTITY(TestEntity, Entity, 0)
public:

  TestVector in;
  TestVector reciprocal;
  };

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

#endif // COMPUTETEST_H
