#ifndef COMPUTETEST_H
#define COMPUTETEST_H

#include <QtTest>
#include "XCore.h"
#include "Memory/XGlobalAllocator.h"
#include "shifttestcommon.h"
#include "shift/sentity.h"
#include "shift/TypeInformation/smodule.h"
#include "shift/Properties/sarray.h"
#include "shift/Properties/sbaseproperties.h"

// future tests
// casting test, benchmarking shallow and deep hierarchies
// iteration test, and benchmarks, including typing...

class ShiftCoreTest : public QObject
  {
  Q_OBJECT

public:
  ShiftCoreTest() : registry(core.defaultAllocator())
    {
    registry.installModule(Test::shiftModule());
    }

  ~ShiftCoreTest()
    {
    }

private Q_SLOTS:
  void tearDownTest();
  void dataTest();
  void createDestroyTest();
  void insertRemoveTest();
  void reparentTest();
  void indexedChildTest();
  void simpleOperationTest();
  void simpleDirtyCompute();
  void entityCompute();
  void serialisationCopyTest();
  void serialisationJsonTest();
  void deserialisationJsonTest();
  void initialiseTest();
  void initialiseProfileTest();
  void initialiseProfileTest_data();

private:
  Eks::Core core;
  Shift::TypeRegistry registry;
  };

class TestVector : public Shift::Container
  {
  S_ENTITY(TestVector, Container)

public:
  enum { StaticChildMode = Shift::NamedChildren | Container::StaticChildMode };

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

class TestIndexedEntity : public Shift::Entity
  {
  S_ENTITY(TestIndexedEntity, Entity)

public:
  Shift::Array testArray;
  };


#endif // COMPUTETEST_H
