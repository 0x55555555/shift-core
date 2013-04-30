#include "shifttest.h"
#include "shift/Properties/sdata.inl"

void ShiftCoreTest::simpleDirtyCompute()
  {
  TestDatabase db;

  // create an entity, everything should be dirty
  TestEntity* a = db.addChild<TestEntity>();
  QCOMPARE(a->evaluationCount, 0);
  QCOMPARE(a->in.allDirty(), true);
  QCOMPARE(a->reciprocal.allDirty(), true);

  // get a->in, shouldnt change anything but itself
  a->in.preGet();
  QCOMPARE(a->in.countDirty(), 3);

  // clean components, should only affect themselves
  a->in.x.preGet();
  QCOMPARE(a->in.countDirty(), 2);
  a->in.y.preGet();
  QCOMPARE(a->in.countDirty(), 1);
  a->in.z.preGet();
  QCOMPARE(a->in.countDirty(), 0);


  // get reciprocal, should compute children
  QCOMPARE(a->evaluationCount, 0);
  a->reciprocal.preGet();
  QCOMPARE(a->evaluationCount, 1);
  QCOMPARE(a->in.anyDirty(), false);
  QCOMPARE(a->reciprocal.anyDirty(), false);



  TestEntity* b = db.addChild<TestEntity>();
  QCOMPARE(b->evaluationCount, 0);
  QCOMPARE(b->in.allDirty(), true);
  QCOMPARE(b->reciprocal.allDirty(), true);

  b->in.preGet();
  QCOMPARE(b->evaluationCount, 0);
  b->reciprocal.preGet();
  QCOMPARE(a->evaluationCount, 1);
  QCOMPARE(b->evaluationCount, 1);
  QCOMPARE(b->in.allDirty(), false);
  QCOMPARE(b->reciprocal.allDirty(), false);

  QCOMPARE(a->evaluationCount, 1);
  QCOMPARE(b->evaluationCount, 1);
  b->in.setInput(&a->reciprocal);
  QCOMPARE(b->evaluationCount, 1);
  QCOMPARE(a->evaluationCount, 1);
  QCOMPARE(a->in.anyDirty(), false);
  // after a connection, mainly due to efficient, the from and to are dirty, even thought only the to really requires it...
  QCOMPARE(a->reciprocal.allDirty(), true);
  QCOMPARE(b->in.allDirty(), true);

  // get b->in, shouldnt change anything but itself
  QCOMPARE(a->evaluationCount, 1);
  QCOMPARE(b->evaluationCount, 1);
  b->in.preGet();
  QCOMPARE(a->evaluationCount, 2);
  QCOMPARE(b->evaluationCount, 1);
  QCOMPARE(a->in.countDirty(), 0);
  QCOMPARE(b->in.countDirty(), 0);

  QCOMPARE(a->evaluationCount, 2);
  QCOMPARE(b->evaluationCount, 1);
  b->reciprocal.preGet();
  QCOMPARE(a->evaluationCount, 2);
  QCOMPARE(b->evaluationCount, 2);

  a->in.x = 1;
  QCOMPARE(a->in.countDirty(), 0);
  QCOMPARE(b->in.countDirty(), 4);
  a->in.y = 2;
  QCOMPARE(a->in.countDirty(), 0);
  QCOMPARE(b->in.countDirty(), 4);
  a->in.z = 3;
  QCOMPARE(a->in.countDirty(), 0);
  QCOMPARE(b->in.countDirty(), 4);

  QCOMPARE(a->evaluationCount, 2);
  QCOMPARE(b->evaluationCount, 2);
  b->reciprocal.preGet();
  QCOMPARE(a->evaluationCount, 3);
  QCOMPARE(b->evaluationCount, 3);

  a->in.x = 1;
  QCOMPARE(a->in.countDirty(), 0);
  QCOMPARE(b->in.countDirty(), 4);
  a->in.y = 2;
  QCOMPARE(a->in.countDirty(), 0);
  QCOMPARE(b->in.countDirty(), 4);
  a->in.z = 3;
  QCOMPARE(a->in.countDirty(), 0);
  QCOMPARE(b->in.countDirty(), 4);

  QCOMPARE(a->evaluationCount, 3);

  // clean components, should siblings
  b->in.x.preGet();
  QCOMPARE(a->in.countDirty(), 0);
  b->in.y.preGet();
  QCOMPARE(a->in.countDirty(), 0);
  b->in.z.preGet();
  QCOMPARE(a->in.countDirty(), 0);

  QCOMPARE(a->in.anyDirty(), false);
  QCOMPARE(a->reciprocal.anyDirty(), false);
  QCOMPARE(b->in.anyDirty(), false);

  QCOMPARE(a->evaluationCount, 4);
  QCOMPARE(b->evaluationCount, 3);
  b->reciprocal.preGet();
  QCOMPARE(a->evaluationCount, 4);
  QCOMPARE(b->evaluationCount, 4);
  }

void ShiftCoreTest::entityCompute()
  {
  TestDatabase db;

  // create an entity, everything should be dirty
  TestEntity* a = db.addChild<TestEntity>();
  TestEntity* b = db.addChild<TestEntity>();

  QCOMPARE(a->evaluationCount, 0);
  QCOMPARE(b->evaluationCount, 0);

  b->setInput(a);

  QCOMPARE(a->evaluationCount, 0);
  QCOMPARE(b->evaluationCount, 0);
  }

QTEST_APPLESS_MAIN(ShiftCoreTest)
