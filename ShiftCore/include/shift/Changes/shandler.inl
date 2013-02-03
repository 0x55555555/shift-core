#ifndef SHANDLERIMPL_H
#define SHANDLERIMPL_H

#include "shift/sdatabase.h"

namespace Shift
{

#ifdef X_CPPOX_VARIADIC_TEMPLATES_SUPPORT
template <typename CLS, typename... CLSARGS> void Handler::doChange(CLSARGS&&... params)
  {
  SProfileFunction
  StateStorageBlock ss(false, this);

  if(!oldStateStorageEnabled)
    {
    CLS change(std::forward<CLSARGS>(params)...);
    ((Change&)change).apply();
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
    Change* change = new(mem) CLS(std::forward<CLSARGS>(params)...);

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
  }
#else
#define DO_CHANGE_IMPL(...) { \
  SProfileFunction \
  StateStorageBlock ss(false, this); \
  if(!oldStateStorageEnabled) { \
    CLS change(__VA_ARGS__); \
    ((Change&)change).apply(); \
    ((Change&)change).inform(false); \
  }else { \
    QMutexLocker l(&_doChange); \
    void *mem = changeAllocator()->alloc(sizeof(CLS)); \
    Change* change = new(mem) CLS(__VA_ARGS__); \
    bool result = change->apply() && change->inform(false); \
    if(result) { \
      _done << change; \
    } else { \
      xAssertFailMessage("Change failed"); \
    } } \
  if(_blockLevel == 0) { inform(); } \
  }

template <typename CLS, typename T0> void Handler::doChange(const T0 &t0)
  DO_CHANGE_IMPL(t0)
template <typename CLS, typename T0, typename T1> void Handler::doChange(const T0 &t0, const T1 &t1)
  DO_CHANGE_IMPL(t0, t1)
template <typename CLS, typename T0, typename T1, typename T2> void Handler::doChange(const T0 &t0, const T1 &t1, const T2 &t2)
  DO_CHANGE_IMPL(t0, t1, t2)
template <typename CLS, typename T0, typename T1, typename T2, typename T3> void Handler::doChange(const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3)
  DO_CHANGE_IMPL(t0, t1, t2, t3)
#endif

}

#endif // SHANDLERIMPL_H
