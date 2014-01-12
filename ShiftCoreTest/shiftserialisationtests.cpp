#include "shifttest.h"
#include "shift/Properties/scontainer.inl"
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

bool checkStrings(const QString &actualIn, const QString &expectedIn)
  {
  xsize lines = 1;
  xsize lineChars = 0;

  xsize linesContext = 3;

  QString actual = actualIn;
  actual.replace('\r', "");
  QString expected = expectedIn;
  expected.replace('\r', "");

  auto context = [linesContext](const QString &str, int pos)
    {
    auto skipLines = [](const QString &str, int pos, xsize linesToGo, int dir)
      {
      while(linesToGo && pos > 0)
        {
        if(str[pos] == '\n')
          {
          --linesToGo;
          }

        pos += dir;
        }

      return pos;
      };

    int start = skipLines(str, pos, linesContext + 1, -1);
    int end = skipLines(str, pos, linesContext + 1, 1);


    return str.mid(start, end-start);
    };

  for(int i = 0; i < std::max(actual.length(), expected.length()); ++i)
    {
    ++lineChars;
    auto charA = i < actual.length() ? ((const QString &)actual)[i] : '\0';
    auto charE = i < expected.length() ? ((const QString &)expected)[i] : '\0';

    if (charA != charE)
      {
      qWarning().nospace() << "Character difference at line " << lines << ", character " << lineChars;
      qWarning().nospace() << "Expected '" << charE << "' (" << charE.unicode() << "), got '" << charA << "' (" << charA.unicode() << ")";

      qWarning() << "Expected:" << context(expected, i);

      qWarning() << "Actual:" << context(actual, i);

      return false;
      }

    if (charA == '\n')
      {
      lineChars = 0;
      ++lines;
      }
    }

  return true;
  }

bool checkProperties(Shift::Attribute *a, Shift::Attribute *aRoot, Shift::Attribute *b, Shift::Attribute *bRoot)
  {
  if (a == nullptr && b == nullptr)
    {
    return true;
    }

  if(!!a != !!b)
    {
    qDebug() << "Existance mismatch";
    return false;
    }

  if(a->typeInformation() != b->typeInformation())
    {
    qDebug() << "Type mismatch";
    return false;
    }

  auto aRelPath = a->path(aRoot, Eks::Core::globalAllocator());
  auto bRelPath = b->path(bRoot, Eks::Core::globalAllocator());
  if(aRelPath != bRelPath)
    {
    qDebug() << "Path mismatch";
    return false;
    }

  if(a->baseInstanceInformation()->mode() != b->baseInstanceInformation()->mode())
    {
    qDebug() << "Mode mismatch";
    return false;
    }

  if(a->isDynamic() != b->isDynamic())
    {
    qDebug() << "Dynamic mismatch";
    return false;
    }

  return true;
  }

xsize checkHierarchies(Shift::Attribute *a, Shift::Attribute *b, bool includeRoot)
  {
  using namespace Shift::Iterator;

  ChildTree aRange;
  ChildTree bRange;
  aRange.reset(a);
  bRange.reset(b);

  auto aIt = aRange.begin();
  auto bIt = bRange.begin();

  xsize checked = 0;
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
      return checked;
      }

    Shift::Property *aProp = aAttr->castTo<Shift::Property>();
    Shift::Property *bProp = bAttr->castTo<Shift::Property>();
    if(!!aProp != !!bProp)
      {
      qDebug() << "Input mismatch";
      return checked;
      }

    if(!aProp)
      {
      continue;
      }

    auto aInput = aProp->input();
    auto bInput = bProp->input();

    if(!checkProperties(aInput, a, bInput, b))
      {
      return checked;
      }
    ++checked;
    }

  bool aValid = aIt != aRange.end();
  bool bValid = bIt != bRange.end();
  if(aValid != bValid)
    {
    qWarning() << "Tree Size Mismatch";
    return 0;
    }

  return checked;
  }

void ShiftCoreTest::serialisationCopyTest()
  {
  TestDatabase db;

  auto rootA = db.addChild<Shift::Entity>();
  buildTestData(rootA);

  Shift::SaveBuilder saver;
  Shift::LoadBuilder loader;

  Shift::Entity *rootB = nullptr;
  QBENCHMARK {
    {
    rootB = db.addChild<Shift::Entity>();

    Eks::TemporaryAllocator alloc(rootB->temporaryAllocator());
    auto loading = loader.beginLoading(rootB, &alloc);

    saver.save(rootA, false, &loader);
    }

    QCOMPARE(checkHierarchies(rootA, rootB, false), 30U);
  }
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

#define SAVE_OUTPUTx
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

  QCOMPARE(checkStrings(savedOutput, expectedOutput), true);
  }

void ShiftCoreTest::deserialisationJsonTest()
  {
  TestDatabase db;

  auto rootA = db.addChild<Shift::Entity>();
  buildTestData(rootA);

  Shift::Entity *rootB = nullptr;
  QBENCHMARK {
    rootB = db.addChild<Shift::Entity>();

    QFile toLoad(":/Serialisation/SerialisationTest.json");
    QCOMPARE(true, toLoad.open(QFile::ReadOnly));

    Shift::LoadBuilder builder;
    Eks::TemporaryAllocator alloc(rootB->temporaryAllocator());
    auto loading = builder.beginLoading(rootB, &alloc);

    Shift::JSONLoader loader;
    loader.load(&toLoad, &builder);
  }

  QCOMPARE(checkHierarchies(rootA, rootB, false), 30U);
  }
