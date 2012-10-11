#ifndef SPROPERTY_INL
#define SPROPERTY_INL

#include "sproperty.h"
#include "spropertyinstanceinformation.h"
#include "spropertyinformation.h"

inline bool SProperty::isDynamic() const
  {
  SProfileFunction
  return baseInstanceInformation()->isDynamic();
  }

inline const SPropertyInformation *SProperty::typeInformation() const
  {
  return _instanceInfo->childInformation();
  }

inline const SProperty::BaseInstanceInformation *SProperty::baseInstanceInformation() const
  {
  xAssert(_instanceInfo);
  return _instanceInfo;
  }

inline const SProperty::EmbeddedInstanceInformation *SProperty::embeddedBaseInstanceInformation() const
  {
  xAssert(_instanceInfo);
  return _instanceInfo->embeddedInfo();
  }
inline const SProperty::DynamicInstanceInformation *SProperty::dynamicBaseInstanceInformation() const
  {
  xAssert(_instanceInfo);
  return _instanceInfo->dynamicInfo();
  }


template <typename T> const SEmbeddedPropertyInstanceInformation *SPropertyInformation::firstChild() const
  {
  const SPropertyInformation *info = T::staticTypeInformation();
  const SEmbeddedPropertyInstanceInformation *first = firstChild();
  while(first)
    {
    const SPropertyInformation *firstInfo = first->childInformation();
    if(firstInfo->inheritsFromType(info))
      {
      return first;
      }
    first = first->nextSibling();
    }
  return 0;
  }

template <typename T> const SEmbeddedPropertyInstanceInformation *SEmbeddedPropertyInstanceInformation::nextSibling() const
  {
  const SPropertyInformation *info = T::staticTypeInformation();
  const SEmbeddedPropertyInstanceInformation *next = _nextSibling;
  while(next)
    {
    const SPropertyInformation *nextInfo = next->childInformation();
    if(nextInfo->inheritsFromType(info))
      {
      return next;
      }
    next = next->nextSibling();
    }
  return 0;
  }

#endif // SPROPERTY_INL
