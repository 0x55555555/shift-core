#ifndef COMPUTETEST_H
#define COMPUTETEST_H

#include <QtTest>
#include "XCore"

class ShiftCoreTest : public QObject
  {
  Q_OBJECT

public:
  ShiftCoreTest();
  ~ShiftCoreTest();

private Q_SLOTS:
  void simpleDirtyCompute();

private:
  Eks::Core core;
  };

#endif // COMPUTETEST_H
