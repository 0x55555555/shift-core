#include "shifttest.h"
#include "shift/Properties/sbaseproperties.inl"

void ShiftCoreTest::simpleOperationTest()
  {
  TestDatabase db;

  TestEntity* a = db.addChild<TestEntity>();

  TestEntity* b = db.addChild<TestEntity>();

  QCOMPARE(a->in.x.input(), nullptr);

  a->in.x.setInput(&b->reciprocal.x);
  QCOMPARE(a->in.x.input(), &b->reciprocal.x);

  a->in.x.setInput(&b->reciprocal.y);
  QCOMPARE(a->in.x.input(), &b->reciprocal.y);

  a->in.x.setInput(0);
  QCOMPARE(a->in.x.input(), nullptr);
  }

void ShiftCoreTest::dataTest()
  {
  TestDatabase db;

  TestEntity* a = db.addChild<TestEntity>();

  Shift::Attribute* attr = &a->in.x;
  
  Shift::Data<float> *f = attr->castTo<Shift::Data<float>>();
  Shift::Data<xint32> *i = attr->castTo<Shift::Data<xint32>>();
  QCOMPARE(attr, f);
  QCOMPARE(nullptr, i);

  xsize successful = 0;
  xsize count = 0;

  // 30'000 or so iterations
  QBENCHMARK {
    ++count;
    successful += attr->castTo<Shift::Data<float>>() ? 1 : 0;

    successful += attr->castTo<Shift::Data<xint32>>() ? 1 : 0;

    successful += a->castTo<TestEntity>() ? 1 : 0;
    successful += a->castTo<Shift::Array>() ? 1 : 0;
    successful += a->castTo<Shift::Container>() ? 1 : 0;
  }

  QCOMPARE(3, successful/count);
  }
