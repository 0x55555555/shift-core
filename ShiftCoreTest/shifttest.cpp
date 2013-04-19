#include "shifttest.h"
#include <QString>
#include "shift/sentity.h"
#include "shift/sdatabase.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/TypeInformation/spropertygroup.h"
#include "shift/Properties/sbaseproperties.h"
#include "XGlobalAllocator"

// future tests
// casting test, benchmarking shallow and deep hierarchies
// iteration test, and benchmarks, including typing...

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

namespace Test
{
Shift::PropertyGroup &propertyGroup()
  {
  static Shift::PropertyGroup grp;
  return grp;
  }
}

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

S_IMPLEMENT_PROPERTY(TestVector, Test)

void TestVector::createTypeInformation(
    Shift::PropertyInformationTyped<TestVector> *info,
    const Shift::PropertyInformationCreateData &data)
  {
  if(data.registerAttributes)
    {
    auto childBlock = info->createChildrenBlock(data);

    childBlock.add(&TestVector::x, "x");
    childBlock.add(&TestVector::y, "y");
    childBlock.add(&TestVector::z, "z");
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

    auto a = childBlock.add(&TestEntity::in, "in");
    auto b = childBlock.add(&TestEntity::reciprocal, "reciprocal");

    auto affects = childBlock.createAffects(&b, 1);

    a->setAffects(affects, true);
    b->setCompute([](TestEntity *ent)
      {
      (void)ent;
      });
    }
  }

ShiftCoreTest::ShiftCoreTest()
  {
  Shift::TypeRegistry::initiate(core.defaultAllocator());
  }

ShiftCoreTest::~ShiftCoreTest()
  {
  Shift::TypeRegistry::terminate();
  }

void ShiftCoreTest::simpleDirtyCompute()
  {
  Shift::Database db;

  // create an entity, everything should be dirty
  TestEntity* a = db.addChild<TestEntity>();
  QCOMPARE(a->in.allDirty(), true);
  QCOMPARE(a->reciprocal.allDirty(), true);

  // get a->in, shouldnt change anything but itself
  a->in.preGet();
  QCOMPARE(a->in.countDirty(), 3);

  // clean components, shoudl only affect themselves
  a->in.x.preGet();
  QCOMPARE(a->in.countDirty(), 2);
  a->in.y.preGet();
  QCOMPARE(a->in.countDirty(), 1);
  a->in.z.preGet();
  QCOMPARE(a->in.countDirty(), 0);


  // get reciprocal, should compute children
  a->reciprocal.preGet();
  QCOMPARE(a->reciprocal.anyDirty(), false);



  TestEntity* b = db.addChild<TestEntity>();
  QCOMPARE(b->in.allDirty(), true);
  QCOMPARE(b->reciprocal.allDirty(), true);

  // get b->in, shouldnt change anything but itself
  b->in.preGet();
  QCOMPARE(a->in.countDirty(), 3);

  // clean components, should only affect themselves
  b->in.x.preGet();
  QCOMPARE(a->in.countDirty(), 2);
  b->in.y.preGet();
  QCOMPARE(a->in.countDirty(), 1);
  b->in.z.preGet();
  QCOMPARE(a->in.countDirty(), 0);

  b->in.setInput(&a->reciprocal);
  QCOMPARE(a->reciprocal.anyDirty(), false);
  QCOMPARE(b->in.allDirty(), true);

  b->in.x.preGet();
  QCOMPARE(a->in.anyDirty(), false);
  QCOMPARE(a->reciprocal.anyDirty(), false);
  QCOMPARE(b->in.anyDirty(), false);
  }

QTEST_APPLESS_MAIN(ShiftCoreTest)
