#ifndef SHANDLER_H
#define SHANDLER_H

#include "shift/sglobal.h"
#include "shift/TypeInformation/sinterface.h"

namespace Shift
{

class Database;
class Change;


class SHIFT_EXPORT Handler : public InterfaceBase
  {
  S_INTERFACE_TYPE(HandlerInterface)

XProperties:
  XWOProperty(Database *, database, setDatabase);

public:
  Handler();
  ~Handler();

  void clearChanges();

  static Handler *findHandler(PropertyContainer *parent, Property *prop);

  void beginBlock();
  void endBlock(bool cancel = false);


  Database *database() { xAssert(_database); return _database; }
  const Database *database() const { xAssert(_database); return _database; }

#ifdef X_CPPOX_VARIADIC_TEMPLATES_SUPPORT
  template <typename CLS, typename... CLSARGS> void doChange(CLSARGS&&... params);
#else
  template <typename CLS, typename T0> void doChange(const T0 &t0);
  template <typename CLS, typename T0, typename T1> void doChange(const T0 &t0, const T1 &t1);
  template <typename CLS, typename T0, typename T1, typename T2> void doChange(const T0 &t0, const T1 &t1, const T2 &t2);
  template <typename CLS, typename T0, typename T1, typename T2, typename T3> void doChange(const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3);
#endif

  Observers &currentBlockObserverList() { return _blockObservers; }

private:
  Eks::AllocatorBase *changeAllocator();

  xuint32 _blockLevel;

  void undoTo(xsize p);

  void inform();
  Observers _blockObservers;
  QMutex _doChange;

  Eks::Vector <Change*> _done;
  Eks::Vector <xsize> _blockSize;
  };

class SHIFT_EXPORT Block
  {
public:
  Block(Handler *db) : _db(db)
    {
#ifdef S_CENTRAL_CHANGE_HANDLER
    xAssert(_db);
    _db->beginBlock();
#endif
    }

  ~Block()
    {
#ifdef S_CENTRAL_CHANGE_HANDLER
    _db->endBlock();
#endif
    }

private:
  Handler *_db;
  };

}

#endif // SHANDLER_H
