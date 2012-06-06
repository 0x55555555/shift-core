#include "sobserver.h"
#include "sentity.h"

SObserver::~SObserver()
  {
  foreach(SEntity *ent, _entities)
    {
    ent->removeObserver(this);
    }
  _entities.clear();
  }
