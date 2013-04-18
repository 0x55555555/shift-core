#ifndef COMPUTETEST_H
#define COMPUTETEST_H

#include <QtTest>

class ShiftCoreComputeTest : public QObject
  {
  Q_OBJECT

public:
  ShiftCoreComputeTest();

private Q_SLOTS:
  void simpleDirty();
  };

#endif // COMPUTETEST_H
