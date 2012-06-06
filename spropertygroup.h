#ifndef SPROPERTYGROUP_H
#define SPROPERTYGROUP_H

#include "sglobal.h"
#include "XProperty"
#include "QVector"

class SPropertyInformation;

class SHIFT_EXPORT SPropertyGroup
  {
public:
  typedef const SPropertyInformation *(*BootstrapFunction)();
  struct Information
    {
    SPropertyInformation *information;
    BootstrapFunction bootstrap;
    Information *next;
    };

  SPropertyGroup();

  void bootstrap();
  Information registerPropertyInformation(Information *info, BootstrapFunction fn);

private:
  Information *_first;

  friend class STypeRegistry;
  };

#endif // SPROPERTYGROUP_H
