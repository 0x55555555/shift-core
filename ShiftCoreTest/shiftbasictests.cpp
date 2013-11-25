#include "shifttest.h"
#include "shift/Properties/sdata.inl"
#include "shift/Properties/scontaineriterators.h"

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

void ShiftCoreTest::tearDownTest()
  {
  TestDatabase db;

  QBENCHMARK {
    Shift::Entity *e = &db;
    for(xsize i = 0; i < 20; ++i)
      {
      auto newA = e->addChild<TestEntity>();
      auto newB = e->addChild<TestEntity>();
      auto newC = e->addChild<TestEntity>();


      newA->in.setInput(e);
      QCOMPARE(newA->in.input(), e);
      newB->in.setInput(e);
      QCOMPARE(newB->in.input(), e);
      newC->in.setInput(e);
      QCOMPARE(newC->in.input(), e);
      
      QCOMPARE(e->output(), &newA->in);
      QCOMPARE(e->output()->nextOutput(), &newB->in);
      QCOMPARE(e->output()->nextOutput()->nextOutput(), &newC->in);
      QCOMPARE(e->output()->nextOutput()->nextOutput()->nextOutput(), nullptr);

      QCOMPARE(e->outputsTo(&newA->in), true);
      QCOMPARE(e->outputsTo(&newB->in), true);
      QCOMPARE(e->outputsTo(&newC->in), true);

      newA->in.disconnect();
      QCOMPARE(newA->in.input(), nullptr);
      QCOMPARE(e->outputsTo(&newA->in), false);
      QCOMPARE(e->outputsTo(&newB->in), true);
      QCOMPARE(e->outputsTo(&newC->in), true);
      newC->in.disconnect();
      QCOMPARE(newC->in.input(), nullptr);
      QCOMPARE(e->outputsTo(&newA->in), false);
      QCOMPARE(e->outputsTo(&newB->in), true);
      QCOMPARE(e->outputsTo(&newC->in), false);

      QCOMPARE(e->output(), &newB->in);
      QCOMPARE(e->output()->nextOutput(), nullptr);

      newA->in.setInput(e);
      QCOMPARE(newA->in.input(), e);
      newC->in.setInput(e);
      QCOMPARE(newB->in.input(), e);

      QCOMPARE(e->output(), &newB->in);
      QCOMPARE(e->output()->nextOutput(), &newA->in);
      QCOMPARE(e->output()->nextOutput()->nextOutput(), &newC->in);
      QCOMPARE(e->output()->nextOutput()->nextOutput()->nextOutput(), nullptr);

      e = newA;
      }

    db.children.clear();
    QCOMPARE(db.output(), nullptr);
    }
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

void ShiftCoreTest::createDestroyTest()
  {
  TestDatabase db;

  xsize actionCount = 0;
  xsize count = 0;

  QBENCHMARK {
    ++count;
    for(xsize i = 0; i < 100; ++i)
      {
      db.addChild<TestEntity>();
      ++actionCount;
      }

    while(db.children.firstChild())
      {
      db.children.remove(db.children.firstChild());
      ++actionCount;
      }
    }

  QCOMPARE(actionCount/count, 200);

  for(xsize i = 0; i < 20; ++i)
    {
    TestEntity *ent = db.addChild<TestEntity>();
    QCOMPARE(db.children.at(i), ent);
    QCOMPARE(db.children.index(ent), i);
    }

  {
  xsize i = 0;
  xForeach(auto ent, db.children.walker())
    {
    QCOMPARE(db.children.index(ent), i++);
    }
  }

  db.children.remove(db.children.at(4));
  db.children.remove(db.children.at(10));
  db.children.remove(db.children.at(17));

  {
  xsize i = 0;
  xForeach(auto ent, db.children.walker())
    {
    QCOMPARE(db.children.index(ent), i++);
    }
  }

  Shift::Entity *e1 = db.children.add<TestEntity>(3);
  QCOMPARE(db.children.index(e1), 3);
  Shift::Entity *e2 = db.children.add<TestEntity>(6);
  QCOMPARE(db.children.index(e2), 6);
  Shift::Entity *e3 = db.children.add<TestEntity>(15);
  QCOMPARE(db.children.index(e3), 15);

  while(db.children.firstChild())
    {
    db.children.remove(db.children.firstChild());

    xsize i = 0;
    xForeach(auto ent, db.children.walker())
      {
      QCOMPARE(db.children.index(ent), i++);
      }
    }
  }

void ShiftCoreTest::insertRemoveTest()
  {
  TestDatabase db;

  auto a = db.children.add<TestEntity>(0);
  QCOMPARE(db.children.index(a), 0);

  auto b = db.children.add<TestEntity>(0);
  QCOMPARE(db.children.index(a), 1);
  QCOMPARE(db.children.index(b), 0);

  auto c = db.children.add<TestEntity>(1);
  QCOMPARE(db.children.index(a), 2);
  QCOMPARE(db.children.index(b), 0);
  QCOMPARE(db.children.index(c), 1);


  auto a2 = db.addProperty<TestEntity>(1);
  QCOMPARE(db.index(a), 1);

  auto b2 = db.addProperty<TestEntity>(1);
  QCOMPARE(db.index(a), 2);
  QCOMPARE(db.index(b), 1);

  auto c2 = db.addProperty<TestEntity>(2);
  QCOMPARE(db.index(a), 3);
  QCOMPARE(db.index(b), 1);
  QCOMPARE(db.index(c), 2);
  }
