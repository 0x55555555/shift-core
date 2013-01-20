#ifndef SPROPERTYGROUP_H
#define SPROPERTYGROUP_H

#include "XProperty"
#include "shift/sglobal.h"

namespace Shift
{

class PropertyInformation;

class SHIFT_EXPORT PropertyGroup
  {
public:
  typedef const PropertyInformation *(*BootstrapFunction)(Eks::AllocatorBase *allocator);
  struct Information
    {
    PropertyInformation *information;
    BootstrapFunction bootstrap;
    Information *next;
    };

  PropertyGroup();

  void bootstrap(Eks::AllocatorBase *allocator);
  Information registerPropertyInformation(Information *info, BootstrapFunction fn);

private:
  Information *_first;

  friend class TypeRegistry;
  };

}

#endif // SPROPERTYGROUP_H
