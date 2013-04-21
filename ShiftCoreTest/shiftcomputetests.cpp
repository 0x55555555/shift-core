#include "shifttest.h"

void ShiftCoreTest::simpleDirtyCompute()
  {
  TestDatabase db;

  // create an entity, everything should be dirty
  TestEntity* a = db.addChild<TestEntity>();
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
  a->reciprocal.preGet();
  QCOMPARE(a->in.anyDirty(), false);
  QCOMPARE(a->reciprocal.anyDirty(), false);



  TestEntity* b = db.addChild<TestEntity>();
  QCOMPARE(b->in.allDirty(), true);
  QCOMPARE(b->reciprocal.allDirty(), true);
  
  b->in.preGet();
  b->reciprocal.preGet();
  QCOMPARE(b->in.allDirty(), false);
  QCOMPARE(b->reciprocal.allDirty(), false);

  b->in.setInput(&a->reciprocal);
  QCOMPARE(a->in.anyDirty(), false);
  // after a connection, mainly due to efficient, the from and to are dirty, even thought only the to really requires it...
  QCOMPARE(a->reciprocal.allDirty(), true); 
  QCOMPARE(b->in.allDirty(), true);

  // get b->in, shouldnt change anything but itself
  b->in.preGet();
  QCOMPARE(a->in.countDirty(), 0);
  QCOMPARE(b->in.countDirty(), 0);

  a->in.x = 1;
  QCOMPARE(a->in.countDirty(), 0);
  QCOMPARE(b->in.countDirty(), 4);
  a->in.y = 2;
  QCOMPARE(a->in.countDirty(), 0);
  QCOMPARE(b->in.countDirty(), 4);
  a->in.z = 3;
  QCOMPARE(a->in.countDirty(), 0);
  QCOMPARE(b->in.countDirty(), 4);

  // clean components, should siblings
  b->in.x.preGet();
  QCOMPARE(a->in.countDirty(), 0);
  b->in.y.preGet();
  QCOMPARE(a->in.countDirty(), 0);
  b->in.z.preGet();
  QCOMPARE(a->in.countDirty(), 0);

  b->in.x.preGet();
  QCOMPARE(a->in.anyDirty(), false);
  QCOMPARE(a->reciprocal.anyDirty(), false);
  QCOMPARE(b->in.anyDirty(), false);
  }

QTEST_APPLESS_MAIN(ShiftCoreTest)
