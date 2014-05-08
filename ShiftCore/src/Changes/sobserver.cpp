#include "shift/Changes/sobserver.h"
#include "shift/sentity.h"
#include "shift/Properties/sattribute.inl"

namespace Shift
{

Observer::Observer()
  {
  _entities.allocator() = TypeRegistry::generalPurposeAllocator();
  }

Observer::~Observer()
  {
  xForeach(Entity *ent, _entities)
    {
    ent->removeObserver(this);
    }
  _entities.clear();
  }

}
