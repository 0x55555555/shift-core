#include "spropertygroup.h"
#include "styperegistry.h"

SPropertyGroup::SPropertyGroup() : _first(0)
  {
  }

SPropertyGroup::Information SPropertyGroup::registerPropertyInformation(SPropertyGroup::Information *ths,
                                                              BootstrapFunction bootstrapFunction)
  {
  Information *first = _first;
  _first = ths;

  Information ret = { 0, bootstrapFunction, first };
  return ret;
  }

void SPropertyGroup::bootstrap()
  {
  for(Information *i = _first; i; i = i->next)
    {
    i->bootstrap();

    xAssert(i->information);

    STypeRegistry::addType(i->information);
    }
  }
