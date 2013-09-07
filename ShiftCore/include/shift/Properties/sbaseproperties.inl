#ifndef SBASEPROPERTIES_INL
#define SBASEPROPERTIES_INL

#include "shift/TypeInformation/spropertyinstanceinformation.h"
#include "sdata.inl"

namespace Shift
{
namespace detail
{
class UuidPropertyInstanceInformation : public EmbeddedPropertyInstanceInformation
  {
  typedef QUuid PODType;

public:
  void initiateAttribute(Attribute *propertyToInitiate) const X_OVERRIDE;

  QUuid defaultValue() const
    {
    return QUuid();
    }
  };

template<> class DataEmbeddedInstanceInformation<Data<QUuid, AttributeData>>
    : public UuidPropertyInstanceInformation
  {
public:
  void initiateAttribute(Attribute *propertyToInitiate) const { UuidPropertyInstanceInformation::initiateAttribute(propertyToInitiate); }
  };

template<> class DataEmbeddedInstanceInformation<Data<QUuid, FullData>>
    : public UuidPropertyInstanceInformation
  {
public:
  void initiateAttribute(Attribute *propertyToInitiate) const { UuidPropertyInstanceInformation::initiateAttribute(propertyToInitiate); }
  };
}

}

#endif // SBASEPROPERTIES_INL
