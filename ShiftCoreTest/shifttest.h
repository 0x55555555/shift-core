#ifndef COMPUTETEST_H
#define COMPUTETEST_H

#include <QtTest>
#include "XCore"
#include "XGlobalAllocator"
#include "shifttestcommon.h"
#include "shift/sentity.h"
#include "shift/Properties/sbaseproperties.h"

// future tests
// casting test, benchmarking shallow and deep hierarchies
// iteration test, and benchmarks, including typing...

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
  void tearDownTest();
  void dataTest();
  void createDestroyTest();
  void simpleOperationTest();
  void simpleDirtyCompute();
  void entityCompute();

private:
  Eks::Core core;
  };

class TestVector : public Shift::Container
  {
  S_ENTITY(TestVector, Container)
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
  S_ENTITY(TestEntity, Entity)
public:
  TestEntity()
    {
    evaluationCount = 0;
    }

  TestVector in;
  TestVector reciprocal;

  xsize evaluationCount;
  };


#endif // COMPUTETEST_H
