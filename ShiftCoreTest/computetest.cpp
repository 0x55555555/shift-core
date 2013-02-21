#include <QString>
#include <QtTest>

class ShiftCoreComputeTest : public QObject
  {
  Q_OBJECT

public:
  ShiftCoreComputeTest();

private Q_SLOTS:
  void testCase1();
  };

ShiftCoreComputeTest::ShiftCoreComputeTest()
  {
  }

void ShiftCoreComputeTest::testCase1()
  {
  QVERIFY2(true, "Failure");

  QBENCHMARK {
    }
  }

QTEST_APPLESS_MAIN(ShiftCoreComputeTest)

#include "computetest.moc"
