#include "shift/TypeInformation/spropertygroup.h"
#include "shift/TypeInformation/styperegistry.h"

namespace Shift
{

PropertyGroup::PropertyGroup() : _first(0)
  {
  }

PropertyGroup::Information PropertyGroup::registerPropertyInformation(PropertyGroup::Information *ths,
                                                              BootstrapFunction bootstrapFunction)
  {
  Information *first = _first;
  _first = ths;

  Information ret = { 0, bootstrapFunction, first };
  return ret;
  }

void PropertyGroup::bootstrap()
  {
  SProfileFunction

  for(Information *i = _first; i; i = i->next)
    {
    i->bootstrap();

    xAssert(i->information);

    TypeRegistry::addType(i->information);
    }
  }

}
