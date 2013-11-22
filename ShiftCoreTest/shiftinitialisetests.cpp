#include "shifttest.h"
#include "shift/Properties/sdata.inl"
#include "shift/Properties/scontaineriterators.h"

class TestDataEntity : public Shift::Entity
  {
  S_ENTITY(TestDataEntity, Entity)
public:

  // should not compile without this - as we add a dynamic array.
  enum { HasDynamicChildren = true };

  Shift::Data<QUuid> uuid;
  Shift::Data<float> number;
  Shift::Data<Eks::String> pork;
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

void ShiftCoreTest::initialiseTest()
  {
  TestDatabase db;

  auto ent = db.addChild<TestDataEntity>();

  QVERIFY(ent->uuid() != QUuid());
  QCOMPARE(ent->number(), 300);
  QCOMPARE(ent->pork(), "testTest");

# init a prop with dynamically added object and test the pointers are GUUUUD.
# try with entity, and property both with and without v tables, and multiple inheritance before property.
  }
