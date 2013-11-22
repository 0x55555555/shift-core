#include "shifttest.h"
#include "shift/Properties/sdata.inl"
#include "shift/Properties/scontaineriterators.h"

// THIS SHOULD FAIL TO COMPILE IF ADDED
#define TEST_FAILING_TO_EMBED_DYNAMICx

class TestSized
  {
  xuint32 a;
  double b;
  };

class TestSized2
  {
  xuint32 a;
  };

class TestDataEntity : public TestSized2, public Shift::Entity
  {
  S_ENTITY(TestDataEntity, Entity)
public:

  // should not compile without this - as we add a dynamic array.
  enum { HasDynamicChildren = true };

  Shift::Data<QUuid> uuid;
  Shift::Data<float> number;
  Shift::Data<Eks::String> pork;
  };

class TestEmbeddingEntity : public TestSized, public Shift::Entity
  {
  S_ENTITY(TestEmbeddingEntity, Entity)
public:

#ifdef TEST_FAILING_TO_EMBED_DYNAMIC
  TestDataEntity ent;
#endif
  };

class TestEmbeddingEmbedderEntity : public TestSized, public Shift::Entity
  {
  S_ENTITY(TestEmbeddingEmbedderEntity, Entity)
  };

S_IMPLEMENT_PROPERTY(TestDataEntity, Test)

void TestDataEntity::createTypeInformation(
    Shift::PropertyInformationTyped<TestDataEntity> *info,
    const Shift::PropertyInformationCreateData &data)
  {
  if(data.registerAttributes)
    {
    auto childBlock = info->createChildrenBlock(data);

    childBlock.add(&TestDataEntity::uuid, "uuid");

    childBlock.add<Shift::Array>("dynamicArray");

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
