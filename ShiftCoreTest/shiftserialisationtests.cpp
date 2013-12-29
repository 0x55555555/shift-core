#include "shifttest.h"
#include "shift/Properties/sdata.inl"
#include "shift/Properties/scontaineriterators.h"
#include "shift/Utilities/siterator.h"
#include "shift/Serialisation/sjsonio.h"
#include "shift/Serialisation/sxmlio.h"

void buildTestData(Shift::Entity* root)
  {
  auto ent = root->addChild<TestEntity>();

  auto indexed = ent->addChild<TestIndexedEntity>();

  auto vec1 = ent->addAttribute<TestVector>();
  auto vec2 = ent->addAttribute<TestVector>();

  auto vec3 = indexed->testArray.add<TestVector>();
  auto vec4 = indexed->testArray.add<TestVector>();

  vec1->x = 1.0f;
  vec1->y = 2.0f;
  vec1->z = 3.0f;

  vec3->setInput(vec1);
  vec2->setInput(vec3);

  vec4->x = 4.0f;
  vec4->y = 5.0f;
  vec4->z = 6.2f;
  }

bool checkProperties(Shift::Attribute *a, Shift::Attribute *aRoot, Shift::Attribute *b, Shift::Attribute *bRoot)
  {
  if (a == nullptr && b == nullptr)
    {
    return true;
    }

  if(!!a != !!b)
    {
    return false;
    }

  if(a->typeInformation() != b->typeInformation())
    {
    return false;
    }

  auto aRelPath = a->path(aRoot, Eks::Core::globalAllocator());
  auto bRelPath = b->path(bRoot, Eks::Core::globalAllocator());
  if(aRelPath != bRelPath)
    {
    return false;
    }

  if(a->baseInstanceInformation()->mode() != b->baseInstanceInformation()->mode())
    {
    return false;
    }

  if(a->isDynamic() != b->isDynamic())
    {
    return false;
    }

  return true;
  }

bool checkHierarchies(Shift::Attribute *a, Shift::Attribute *b, bool includeRoot)
  {
  using namespace Shift::Iterator;

  Children aRange;
  Children bRange;
  aRange.reset(a);
  bRange.reset(b);

  auto aIt = aRange.begin();
  auto bIt = bRange.begin();

  for(; aIt != aRange.end() && bIt != bRange.end(); ++aIt, ++bIt)
    {
    auto aAttr = *aIt;
    auto bAttr = *bIt;

    if(a == aAttr && !includeRoot)
      {
      xAssert(b == bAttr);
      continue;
      }

    if(!checkProperties(aAttr, a, bAttr, b))
      {
      return false;
      }

    Shift::Property *aProp = aAttr->castTo<Shift::Property>();
    Shift::Property *bProp = bAttr->castTo<Shift::Property>();
    if(!!aProp != !!bProp)
      {
      }

    if(!aProp)
      {
      continue;
      }

    auto aInput = aProp->input();
    auto bInput = bProp->input();

    if(!checkProperties(aInput, a, bInput, b))
      {
      return false;
      }
    }

  bool aValid = aIt != aRange.end();
  bool bValid = bIt != bRange.end();
  if(aValid != bValid)
    {
    return false;
    }
  return true;
  }

void ShiftCoreTest::serialisationCopyTest()
  {
  TestDatabase db;

  auto rootA = db.addChild<Shift::Entity>();
  auto rootB = db.addChild<Shift::Entity>();
  buildTestData(rootA);

  Shift::SaveBuilder saver;
  Shift::LoadBuilder loader;

    {
    auto loading = loader.beginLoading(rootB);

    saver.save(rootA, false, &loader);
    }

  QCOMPARE(checkHierarchies(rootA, rootB, false), true);
  }


void ShiftCoreTest::serialisationJsonTest()
  {
  TestDatabase db;

  auto rootA = db.addChild<Shift::Entity>();
  buildTestData(rootA);

  QBuffer buffer;

  Shift::SaveBuilder builder;

  Shift::JSONSaver writer;
  writer.setAutoWhitespace(true);

  QBENCHMARK {
    buffer.setData(QByteArray());
    buffer.open(QIODevice::ReadWrite);
    auto block = writer.beginWriting(&buffer);

    builder.save(rootA, false, &writer);
    buffer.close();
  }

#define SAVE_OUTPUT
#ifdef SAVE_OUTPUT
    {
    QFileInfo path("./SerialisationTest.json");
    qDebug() << path.absoluteFilePath();
    QFile output(path.absoluteFilePath());
    QCOMPARE(output.open(QFile::WriteOnly), true);

    output.write(buffer.data());
    }
#endif

  QFile expected(":/Serialisation/SerialisationTest.json");
  QCOMPARE(expected.open(QFile::ReadOnly), true);

  QString savedOutput(buffer.data());
  QString expectedOutput(expected.readAll());

  QCOMPARE(savedOutput, expectedOutput);
  }

void ShiftCoreTest::deserialisationJsonTest()
  {
  TestDatabase db;

  auto rootA = db.addChild<Shift::Entity>();
  buildTestData(rootA);

  //auto rootB = db.addChild<Shift::Entity>();

  /*QBENCHMARK {
    QFile toLoad(":/Serialisation/SerialisationTest.json");
    QCOMPARE(true, toLoad.open(QFile::ReadOnly));

    Shift::LoadBuilder builder;
    builder.setRoot(rootB);

    Shift::JSONLoader loader;

    loader.load(&toLoad, &builder);
  }*/
  }

