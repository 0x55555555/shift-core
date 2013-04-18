#include <QString>
#include <QtTest>
#include "shift/sentity.h"
#include "shift/sdatabase.h"
#include "shift/Properties/sbaseproperties.h"

// future tests
// casting test, benchmarking shallow and deep hierarchies
// iteration test, and benchmarks, including typing,,,

class TestVector : public Shift::Container
  {
  S_ENTITY(TestVector, Container, 0)
public:

  Shift::FloatProperty x;
  Shift::FloatProperty y;
  Shift::FloatProperty z;

  bool allDirty() const
    {
    return countDirty == 4;
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

namespace Test
{
Shift::PropertyGroup &propertyGroup()
  {
  static Shift::PropertyGroup grp;
  return grp;
  }
}

class TestEntity : public Shift::Entity
  {
  S_ENTITY(TestEntity, Entity, 0)
public:

  TestVector in;
  TestVector reciprocal;
  };

S_IMPLEMENT_PROPERTY(TestEntity, Test)

void TestEntity::createTypeInformation(
    Shift::PropertyInformationTyped<TestEntity> *info,
    const Shift::PropertyInformationCreateData &data)
  {
  if(data.registerAttributes)
    {
    auto childBlock = info->createChildrenBlock(data);

    auto a = childBlock->add(&TestEntity::in, "in");
    auto b = childBlock->add(&TestEntity::reciprocal, "reciprocal");

    a->setAffects(b);
    b->setCompute<[](TestEntity *ent)
      {
      }>();
    }
  }

class ShiftCoreComputeTest : public QObject
  {
  Q_OBJECT

public:
  ShiftCoreComputeTest();

private Q_SLOTS:
  void simpleDirty();
  };

ShiftCoreComputeTest::ShiftCoreComputeTest()
  {
  }

void ShiftCoreComputeTest::simpleDirty()
  {
  Shift::Database db;

  // create an entity, everything should be dirty
  TestEntity* a = db.addChild<TestEntity>();
  QCOMPARE(a->in->allDirty(), true);
  QCOMPARE(a->reciprocal->allDirty(), true);

  // get a->in, shouldnt change anything but itself
  a->in->preGet();
  QCOMPARE(a->in->countDirty(), 3);

  // clean components, shoudl only affect themselves
  a->x.preGet();
  QCOMPARE(a->in->countDirty(), 2);
  a->y.preGet();
  QCOMPARE(a->in->countDirty(), 1);
  a->z.preGet();
  QCOMPARE(a->in->countDirty(), 0);


  // get reciprocal, should compute children
  a->reciprocal->preGet();
  QCOMPARE(a->reciprocal->anyDirty(), false);

  // get b->in, shouldnt change anything but itself
  b->in->preGet();
  QCOMPARE(a->in->countDirty(), 3);


  TestEntity* b = _db.addChild<TestEntity>();
  QCOMPARE(b->in->allDirty(), true);
  QCOMPARE(b->reciprocal->allDirty(), true);

  // clean components, should only affect themselves
  b->x.preGet();
  QCOMPARE(a->in->countDirty(), 2);
  b->y.preGet();
  QCOMPARE(a->in->countDirty(), 1);
  b->z.preGet();
  QCOMPARE(a->in->countDirty(), 0);

  b->in.setInput(a->reciprocal);
  QCOMPARE(a->reciprocal->anyDirty(), false);
  QCOMPARE(b->in->allDirty(), true);

  b->in->x->preGet();
  QCOMPARE(a->in->anyDirty(), false);
  QCOMPARE(a->reciprocal->anyDirty(), false);
  QCOMPARE(b->in->anyDirty(), false);
  }

QTEST_APPLESS_MAIN(ShiftCoreComputeTest)

#include "computetest.moc"
