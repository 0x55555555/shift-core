#include "shifttest.h"
#include "shift/Properties/sdata.inl"
#include "shift/Properties/scontaineriterators.h"

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

