#include "sobserver.h"
#include "sentity.h"

SObserver::~SObserver()
  {
  Q_FOREACH(SEntity *ent, _entities)
    {
    ent->removeObserver(this);
    }
  _entities.clear();
  }
