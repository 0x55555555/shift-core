#ifndef SPROPERTY_INL
#define SPROPERTY_INL

#include "sproperty.h"
#include "spropertyinstanceinformation.h"

bool SProperty::isDynamic() const
  {
  SProfileFunction
  return baseInstanceInformation()->isDynamic();
  }

const SPropertyInformation *SProperty::typeInformation() const
  {
  return _instanceInfo->childInformation();
  }

const SProperty::BaseInstanceInformation *SProperty::baseInstanceInformation() const
  {
  xAssert(_instanceInfo);
  return _instanceInfo;
  }

const SProperty::EmbeddedInstanceInformation *SProperty::embeddedBaseInstanceInformation() const
  {
  xAssert(_instanceInfo);
  return _instanceInfo->embeddedInfo();
  }
const SProperty::DynamicInstanceInformation *SProperty::dynamicBaseInstanceInformation() const
  {
  xAssert(_instanceInfo);
  return _instanceInfo->dynamicInfo();
  }

#endif // SPROPERTY_INL
