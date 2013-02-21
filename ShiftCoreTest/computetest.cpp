#include <QString>
#include <QtTest>
#include "shift/sentity.h"
#include "shift/sdatabase.h"
#include "shift/Properties/sbaseproperties.h"

class TestVector : public Shift::PropertyContainer
  {
  S_ENTITY(TestEntity, Entity, 0)
public:

  Shift::FloatProperty x;
  Shift::FloatProperty y;
  Shift::FloatProperty z;

  bool allDirty() const
    {
    return isDirty() && x.isDirty() && y.isDirty() && z.isDirty();
    }

  bool anyDirty() const
    {
    return !(isDirty() | x.isDirty() || y.isDirty() || z.isDirty());
    }
  };

class TestEntity : public Shift::Entity
  {
  S_ENTITY(TestEntity, Entity, 0)
public:

  TestVector in;
  TestVector reciprocal;
  };

class ShiftCoreComputeTest : public QObject
  {
  Q_OBJECT

public:
  ShiftCoreComputeTest();

private Q_SLOTS:
  void testCase1();

private:
  Shift::Database _db;
  };

ShiftCoreComputeTest::ShiftCoreComputeTest()
  {
  }

void ShiftCoreComputeTest::testCase1()
  {
  TestEntity* a = _db.addChild<TestEntity>();
  QCOMPARE(a->allDirty(), true);

  a->preGet();
  QCOMPARE(a->anyDirty(), false);

  b->in.setInput(a->reciprocal);
  TestEntity* b = _db.addChild<TestEntity>();

  c->in.setInput(b->reciprocal);
  TestEntity* c = _db.addChild<TestEntity>();
  }

QTEST_APPLESS_MAIN(ShiftCoreComputeTest)

#include "computetest.moc"
