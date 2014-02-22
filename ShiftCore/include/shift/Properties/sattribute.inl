#ifndef SPROPERTY_INL
#define SPROPERTY_INL

#include "shift/Properties/sproperty.h"
#include "shift/TypeInformation/spropertyinstanceinformation.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "Utilities/XEventLogger.h"

namespace Shift
{

inline bool Attribute::isDynamic() const
  {
  SProfileFunction
  return baseInstanceInformation()->isDynamic();
  }

inline const PropertyInformation *Attribute::typeInformation() const
  {
  return _instanceInfo->childInformation();
  }

inline const Attribute::BaseInstanceInformation *Attribute::baseInstanceInformation() const
  {
  xAssert(_instanceInfo);
  return _instanceInfo;
  }

inline const Attribute::EmbeddedInstanceInformation *Attribute::embeddedBaseInstanceInformation() const
  {
  xAssert(_instanceInfo);
  return _instanceInfo->embeddedInfo();
  }

inline const Attribute::DynamicInstanceInformation *Attribute::dynamicBaseInstanceInformation() const
  {
  xAssert(_instanceInfo);
  return _instanceInfo->dynamicInfo();
  }


template <typename T> const EmbeddedPropertyInstanceInformation *PropertyInformation::firstChild(xsize *idx) const
  {
  *idx = 0;
  return _childCount > 0 ? nextChild<T>(idx) : 0;
  }

template <typename T> const EmbeddedPropertyInstanceInformation *PropertyInformation::nextChild(xsize *i) const
  {
  const PropertyInformation *info = T::staticTypeInformation();
  for(; (*i) < _childCount; ++(*i))
    {
    const EmbeddedPropertyInstanceInformation *next = _childData[*i];

    const PropertyInformation *nextInfo = next->childInformation();
    if(nextInfo->inheritsFromType(info))
      {
      return next;
      }
    }

  *i = Eks::maxFor(*i);
  return 0;
  }

}

#endif // SPROPERTY_INL
