#include "shift/Changes/shandler.h"
#include "shift/sdatabase.h"

namespace Shift
{

Handler::Handler()
  : _database(0),
    _blockLevel(0),
    _stateStorageEnabled(false),
    _blockObservers(TypeRegistry::generalPurposeAllocator()),
    _blockSize(TypeRegistry::generalPurposeAllocator())
  {
  }

Handler::~Handler()
  {
  clearChanges();
  }

void Handler::clearChanges()
  {
  Q_FOREACH(Change *ch, _done)
    {
    changeAllocator()->destroy(ch);
    }

  _done.clear();
  _done.squeeze();

  _blockSize.clear();
  _blockSize.squeeze();
  }

Handler *Handler::findHandler(Container *parent, Entity *prop)
  {
  Handler *handler = prop->findInterface<Handler>();
  if(handler)
    {
    handler->setDatabase(parent->database());
    xAssert(handler->database());
    return handler;
    }
  xAssert(parent->handler()->database());
  return parent->handler();
  }

Eks::AllocatorBase *Handler::changeAllocator()
  {
  return _database->persistantAllocator();
  }

Eks::TemporaryAllocatorCore *Handler::temporaryAllocator() const
  {
  return _database->temporaryAllocator();
  }

Eks::AllocatorBase *Handler::persistentBlockAllocator() const
  {
  return _database->persistantAllocator();
  }

Eks::AllocatorBase *Handler::generalPurposeAllocator() const
  {
  return TypeRegistry::generalPurposeAllocator();
  }

void Handler::beginBlock()
  {
  _blockLevel++;
  _blockSize << _done.size();
  }

void Handler::endBlock(bool cancel)
  {
  xAssert(_blockLevel > 0);
  _blockLevel--;

  xsize previousPoint = _blockSize.popBack();

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

void Handler::undoTo(xsize p)
  {
  xAssert(p <= (xsize)_done.size());
  for(xptrdiff i=(_done.size()-1); i>=(xptrdiff)p; --i)
    {
    Change *c = _done[i];

    bool result = c->unApply() && c->inform(true);
    xAssert(result);

    // todo dont need this here, when undo fully implemented.B
    _done.popBack();
    }
  }

void Handler::inform()
  {
  SProfileFunction
  Q_FOREACH(Observer *obs, _blockObservers)
    {
    obs->actOnChanges();
    }
  _blockObservers.clear();
  }

}
