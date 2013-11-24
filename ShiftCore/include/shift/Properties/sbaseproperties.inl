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
  UuidPropertyInstanceInformation()
    {
    }
  void initiateAttribute(Attribute *propertyToInitiate, AttributeInitialiserHelper *) const X_OVERRIDE;

  QUuid defaultValue() const
    {
    return QUuid();
    }
  };

template<> class DataEmbeddedInstanceInformation<Data<QUuid, AttributeData>>
    : public UuidPropertyInstanceInformation
  {
public:
  DataEmbeddedInstanceInformation()
    {
    }
  };

template<> class DataEmbeddedInstanceInformation<Data<QUuid, FullData>>
    : public UuidPropertyInstanceInformation
  {
public:
  DataEmbeddedInstanceInformation()
    {
    }
  };
}

}

#endif // SBASEPROPERTIES_INL
