#ifndef SHANDLERIMPL_H
#define SHANDLERIMPL_H

#include "sdatabase.h"

#ifdef X_CPPOX_VARIADIC_TEMPLATES_SUPPORT
template <typename CLS, typename... CLSARGS> void SHandler::doChange(CLSARGS&&... params)
  {
  SProfileFunction
  bool oldStateStorageEnabled = database()->stateStorageEnabled();
  database()->setStateStorageEnabled(false);

  if(!oldStateStorageEnabled)
    {
    CLS change(std::forward<CLSARGS>(params)...);
    ((SChange&)change).apply();
    }
  else
    {
#if X_ASSERTS_ENABLED
    if(_doChange.tryLock())
      {
      _doChange.unlock();
      }
    else
      {
      xAssertFail();
      }
#endif
    QMutexLocker l(&_doChange);
    void *mem = changeAllocator()->alloc(sizeof(CLS));
    SChange* change = new(mem) CLS(std::forward<CLSARGS>(params)...);

    bool result = change->apply() && change->inform(false);

    if(result)
      {
      _done << change;
      }
    else
      {
      xAssertFailMessage("Change failed");
      }
    }

  if(_blockLevel == 0)
    {
    inform();
    }

  database()->setStateStorageEnabled(oldStateStorageEnabled);
  }
#else
#define DO_CHANGE_IMPL(...) { \
  SProfileFunction \
  bool oldStateStorageEnabled = database()->stateStorageEnabled(); \
  database()->setStateStorageEnabled(false); \
  if(!oldStateStorageEnabled) { \
    CLS change(__VA_ARGS__); \
    ((SChange&)change).apply(); \
    ((SChange&)change).inform(false); \
  }else { \
    QMutexLocker l(&_doChange); \
    void *mem = changeAllocator()->alloc(sizeof(CLS)); \
    SChange* change = new(mem) CLS(__VA_ARGS__); \
    bool result = change->apply() && change->inform(false); \
    if(result) { \
      _done << change; \
    } else { \
      xAssertFailMessage("Change failed"); \
    } } \
  if(_blockLevel == 0) { inform(); } \
  database()->setStateStorageEnabled(oldStateStorageEnabled); \
  }

template <typename CLS, typename T0> void SHandler::doChange(const T0 &t0)
  DO_CHANGE_IMPL(t0)
template <typename CLS, typename T0, typename T1> void SHandler::doChange(const T0 &t0, const T1 &t1)
  DO_CHANGE_IMPL(t0, t1)
template <typename CLS, typename T0, typename T1, typename T2> void SHandler::doChange(const T0 &t0, const T1 &t1, const T2 &t2)
  DO_CHANGE_IMPL(t0, t1, t2)
template <typename CLS, typename T0, typename T1, typename T2, typename T3> void SHandler::doChange(const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3)
  DO_CHANGE_IMPL(t0, t1, t2, t3)
#endif

#endif // SHANDLERIMPL_H
