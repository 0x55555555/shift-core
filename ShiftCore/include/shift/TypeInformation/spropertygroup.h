#ifndef SPROPERTYGROUP_H
#define SPROPERTYGROUP_H

#include "XProperty"
#include "QVector"
#include "shift/sglobal.h"

namespace Shift
{

class PropertyInformation;

class SHIFT_EXPORT PropertyGroup
  {
public:
  typedef const PropertyInformation *(*BootstrapFunction)();
  struct Information
    {
    PropertyInformation *information;
    BootstrapFunction bootstrap;
    Information *next;
    };

  PropertyGroup();

  void bootstrap();
  Information registerPropertyInformation(Information *info, BootstrapFunction fn);

private:
  Information *_first;

  friend class TypeRegistry;
  };

}

#endif // SPROPERTYGROUP_H
