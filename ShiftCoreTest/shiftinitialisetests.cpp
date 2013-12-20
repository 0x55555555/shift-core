#include "shifttest.h"
#include "shift/Properties/sdata.inl"
#include "shift/Properties/sbaseproperties.inl"
#include "shift/Properties/scontaineriterators.h"
#include <memory>

// THIS SHOULD FAIL TO COMPILE IF ADDED
#define TEST_FAILING_TO_EMBED_DYNAMICx

class TestSized
  {
public:
  xuint32 a;
  double b;
  };

class TestSized2
  {
public:
  xuint32 a;
  };

class TestDataEntity : public TestSized2, public Shift::Entity
  {
  S_ENTITY(TestDataEntity, Entity)
public:

  enum { StaticChildMode = Entity::StaticChildMode | Shift::AllowExtraChildren };

  Shift::Data<QUuid> uuid;
  Shift::Data<float> number;
  Shift::Data<Eks::String> pork;
  };

class TestEmbeddingEntity : public TestSized, public Shift::Entity
  {
  S_ENTITY(TestEmbeddingEntity, Entity)
public:

  enum { StaticChildMode = Entity::StaticChildMode | Shift::AllowExtraChildren };

#ifdef TEST_FAILING_TO_EMBED_DYNAMIC
  TestDataEntity ent;
#endif
  };

class TestEmbeddingEmbedderEntity : public TestSized, public Shift::Entity
  {
  S_ENTITY(TestEmbeddingEmbedderEntity, Entity)
public:

  enum { StaticChildMode = Entity::StaticChildMode | Shift::AllowExtraChildren };

  };

S_IMPLEMENT_PROPERTY(TestDataEntity, Test)

void TestDataEntity::createTypeInformation(
    Shift::PropertyInformationTyped<TestDataEntity> *info,
    const Shift::PropertyInformationCreateData &data)
  {
  if(data.registerAttributes)
    {
    auto childBlock = info->createChildrenBlock(data);

    auto inst = childBlock.add(&TestDataEntity::uuid, "uuid");

    childBlock.add<Shift::Set>("dynamicArray");

    auto num = childBlock.add(&TestDataEntity::number, "number");
    num->setDefaultValue(300);

    auto pork = childBlock.add(&TestDataEntity::pork, "pork");
    pork->setDefaultValue("testTest");
    }
  }

S_IMPLEMENT_PROPERTY(TestEmbeddingEntity, Test)

void TestEmbeddingEntity::createTypeInformation(
    Shift::PropertyInformationTyped<TestEmbeddingEntity> *info,
    const Shift::PropertyInformationCreateData &data)
  {
  if(data.registerAttributes)
    {
    auto childBlock = info->createChildrenBlock(data);

#ifdef TEST_FAILING_TO_EMBED_DYNAMIC
    childBlock.add(&TestEmbeddingEntity::ent, "ent");
#endif

    childBlock.add<TestDataEntity>("extra");
    childBlock.add<TestDataEntity>("extra1");
    childBlock.add<TestDataEntity>("extra2");
    }
  }

S_IMPLEMENT_PROPERTY(TestEmbeddingEmbedderEntity, Test)

void TestEmbeddingEmbedderEntity::createTypeInformation(
    Shift::PropertyInformationTyped<TestEmbeddingEmbedderEntity> *info,
    const Shift::PropertyInformationCreateData &data)
  {
  if(data.registerAttributes)
    {
    auto childBlock = info->createChildrenBlock(data);

    auto a = childBlock.add<TestEmbeddingEntity>("extra");
    childBlock.add<TestEmbeddingEntity>("extra1");
    auto b = childBlock.add<TestEmbeddingEntity>("extra2");
    b->setDefaultInput(a);
    }
  }

void ShiftCoreTest::initialiseTest()
  {
  TestDatabase db;

  auto ent = db.addChild<TestDataEntity>();

  QVERIFY(ent->uuid() != QUuid());
  QCOMPARE(ent->number(), 300);
  QCOMPARE(ent->pork(), "testTest");

  auto testDataInfo = TestDataEntity::staticTypeInformation();
  auto testEmbeddedInfo = TestEmbeddingEntity::staticTypeInformation();
  QCOMPARE(testDataInfo->propertyDataOffset(), xMax(sizeof(TestSized2), sizeof(void*)));
  QCOMPARE(testEmbeddedInfo->propertyDataOffset(), sizeof(TestSized));

  auto emb = db.addChild<TestEmbeddingEntity>();
  QCOMPARE(emb->at(0), &emb->children);
  QCOMPARE(emb->at(1)->name(), "extra");
  QCOMPARE(emb->typeInformation(), testEmbeddedInfo);
  QCOMPARE(emb->at(1)->typeInformation(), testDataInfo);

  auto emb2 = db.addChild<TestEmbeddingEmbedderEntity>();
  QCOMPARE(emb2->at(0)->uncheckedCastTo<Shift::Property>()->input(), nullptr);
  QCOMPARE(emb2->at(2)->uncheckedCastTo<Shift::Property>()->input(),
           emb2->at(0)->uncheckedCastTo<Shift::Property>()->input());

  // investigate post init and insert steps

  // insertion costs?

  // deep default inputs.
  }

void ShiftCoreTest::initialiseProfileTest_data()
  {
  QTest::addColumn<int>("count");
  QTest::addColumn<bool>("timeTearDown");
  QTest::addColumn<bool>("timeDbCtorDtor");
  QTest::addColumn<bool>("optimiseInsert");
  QTest::addColumn<bool>("indexedInsert");

  QTest::newRow("1    A") << 1     << false << false << false << false;
  QTest::newRow("10   A") << 10    << false << false << false << false;
  QTest::newRow("100  A") << 100   << false << false << false << false;
  QTest::newRow("1000 A") << 1000  << false << false << false << false;
#ifndef X_DEBUG
  QTest::newRow("10000A") << 10000 << false << false << false << false;
#endif

  QTest::newRow("1    B") << 1     << true  << false << false << false;
  QTest::newRow("10   B") << 10    << true  << false << false << false;
  QTest::newRow("100  B") << 100   << true  << false << false << false;
  QTest::newRow("1000 B") << 1000  << true  << false << false << false;
#ifndef X_DEBUG
  QTest::newRow("10000B") << 10000 << true  << false << false << false;
#endif

  QTest::newRow("1    C") << 1     << true  << true  << false << false;
  QTest::newRow("10   C") << 10    << true  << true  << false << false;
  QTest::newRow("100  C") << 100   << true  << true  << false << false;
  QTest::newRow("1000 C") << 1000  << true  << true  << false << false;
#ifndef X_DEBUG
  QTest::newRow("10000C") << 10000 << true  << true  << false << false;
#endif

  QTest::newRow("1    D") << 1     << true  << true  << true  << false;
  QTest::newRow("10   D") << 10    << true  << true  << true  << false;
  QTest::newRow("100  D") << 100   << true  << true  << true  << false;
  QTest::newRow("1000 D") << 1000  << true  << true  << true  << false;
#ifndef X_DEBUG
  QTest::newRow("10000D") << 10000 << true  << true  << true  << false;
#endif

  QTest::newRow("1    E") << 1     << true  << true  << false << true ;
  QTest::newRow("10   E") << 10    << true  << true  << false << true ;
  QTest::newRow("100  E") << 100   << true  << true  << false << true ;
  QTest::newRow("1000 E") << 1000  << true  << true  << false << true ;
#ifndef X_DEBUG
  QTest::newRow("10000E") << 10000 << true  << true  << false << true ;
#endif

  }

template <typename T> void doTest(T* arr, int count, bool timeTearDown, bool optimiseInsert)
  {
  Eks::TemporaryAllocator alloc(arr->temporaryAllocator());
  auto editCache = optimiseInsert ? arr->createEditCache(&alloc) : nullptr;
  (void)editCache;

  for(int i = 0; i < count; ++i)
    {
    arr->template add<TestEmbeddingEmbedderEntity>();
    }

  if (timeTearDown)
    {
    arr->clear();
    }
  }

void ShiftCoreTest::initialiseProfileTest()
  {
  QFETCH(int, count);
  QFETCH(bool, timeTearDown);
  QFETCH(bool, timeDbCtorDtor);
  QFETCH(bool, optimiseInsert);
  QFETCH(bool, indexedInsert);

  auto createDb = [timeDbCtorDtor]() { return std::unique_ptr<TestDatabase>(new TestDatabase()); };

  auto db = timeDbCtorDtor ? nullptr : createDb();

  QBENCHMARK {
    if (timeDbCtorDtor)
      {
      db = createDb();
      }

    auto holder = db->addChild<TestIndexedEntity>();

    if(indexedInsert)
      {
      doTest<Shift::Array>(&holder->testArray, count, timeTearDown, optimiseInsert);
      }
    else
      {
      doTest<Shift::Set>(&holder->children, count, timeTearDown, optimiseInsert);
      }

    if (timeDbCtorDtor)
      {
      db = nullptr;
      }
    }
  }
