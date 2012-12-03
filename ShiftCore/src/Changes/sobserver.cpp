#include "shift/Changes/sobserver.h"
#include "shift/sentity.h"

namespace Shift
{

Observer::~Observer()
  {
  Q_FOREACH(Entity *ent, _entities)
    {
    ent->removeObserver(this);
    }
  _entities.clear();
  }

}
