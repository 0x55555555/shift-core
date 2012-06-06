#include "shandler.h"
#include "sdatabase.h"

SHandler::SHandler() : _blockLevel(0), _database(0)
  {
  }

SHandler::~SHandler()
  {
  clearChanges();
  }

void SHandler::clearChanges()
  {
  foreach(SChange *ch, _done)
    {
    xDestroyAndFree(changeAllocator(), SChange, ch);
    }
  _done.clear();
  }

SHandler *SHandler::findHandler(SPropertyContainer *parent, SProperty *prop)
  {
  SHandler *handler = prop->interface<SHandler>();
  if(handler)
    {
    handler->setDatabase(parent->database());
    xAssert(handler->database());
    return handler;
    }
  xAssert(parent->handler()->database());
  return parent->handler();
  }

XAllocatorBase *SHandler::changeAllocator()
  {
  return _database->persistantAllocator();
  }

void SHandler::beginBlock()
  {
  _blockLevel++;
  _blockSize << _done.size();
  }

void SHandler::endBlock(bool cancel)
  {
  xAssert(_blockLevel > 0);
  _blockLevel--;

  xsize previousPoint = _blockSize.last();
  _blockSize.pop_back();
  if(cancel)
    {
    undoTo(previousPoint);
    }

  // wrap everything into one inform block
  if(_blockLevel == 0)
    {
    inform();
    }
  }

void SHandler::undoTo(xsize p)
  {
  xAssert(p <= (xsize)_done.size());
  for(xptrdiff i=(_done.size()-1); i>=(xptrdiff)p; --i)
    {
    SChange *c = _done[i];

    bool result = c->unApply() && c->inform(true);
    xAssert(result);

    // todo dont need this here, when undo fully implemented.B
    _done.pop_back();
    }
  }

void SHandler::inform()
  {
  SProfileFunction
  foreach(SObserver *obs, _blockObservers)
    {
    obs->actOnChanges();
    }
  _blockObservers.clear();
  }


SBlock::SBlock(SHandler *db) : _db(db)
  {
  _db->beginBlock();
  }

SBlock::~SBlock()
  {
  _db->endBlock();
  }
