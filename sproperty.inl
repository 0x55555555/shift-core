#ifndef SPROPERTY_INL
#define SPROPERTY_INL

#include "sproperty.h"
#include "spropertyinstanceinformation.h"

inline const SPropertyInformation *SProperty::typeInformation() const
  {
  return _instanceInfo->childInformation();
  }
const SProperty::StaticInstanceInformation *SProperty::staticBaseInstanceInformation() const
  {
  xAssert(_instanceInfo);
  return _instanceInfo->staticInfo();
  }
const SProperty::DynamicInstanceInformation *SProperty::dynamicBaseInstanceInformation() const
  {
  xAssert(_instanceInfo);
  return _instanceInfo->dynamicInfo();
  }

#endif // SPROPERTY_INL
