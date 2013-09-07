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
  UuidPropertyInstanceInformation() { }

  void initiateAttribute(Attribute *propertyToInitiate) const X_OVERRIDE;

  QUuid defaultValue() const
    {
    return QUuid();
    }
  };

template<> class DataEmbeddedInstanceInformation<Data<QUuid, FullData>>
    : public UuidPropertyInstanceInformation
  {
  };
}

}

#endif // SBASEPROPERTIES_INL
