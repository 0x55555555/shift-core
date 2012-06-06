#ifndef SPROCESSMANAGER_H
#define SPROCESSMANAGER_H

#include "sglobal.h"

class SPropertyContainer;
class SPropertyInstanceInformation;

// this class is internal, do not use it in shift extensions.
class SHIFT_EXPORT SProcessManager
  {
public:
  static void preCompute(const SPropertyInstanceInformation *info, SPropertyContainer *ent);

  static bool isMainThread();

  static void initiate(xsize processes);
  static void terminate();

private:
  SProcessManager();
  X_DISABLE_COPY(SProcessManager);
  };

#endif // SPROCESSMANAGER_H
