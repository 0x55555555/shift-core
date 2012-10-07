#ifndef SPROPERTY_INL
#define SPROPERTY_INL

#include "sproperty.h"
#include "spropertyinstanceinformation.h"

inline const SPropertyInformation *SProperty::typeInformation() const
  {
  return _instanceInfo->childInformation();
  }

#endif // SPROPERTY_INL
