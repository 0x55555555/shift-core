#include "shift/Properties/sdata.h"
#include "shift/Properties/sdata.inl"

namespace Shift
{

void thing()
  {
  detail::PODEmbeddedInstanceInformation<Data<QUuid, FullData>, true> aX;
  detail::PODEmbeddedInstanceInformation2<Data<QUuid, FullData>, true> aY;
  detail::PODEmbeddedInstanceInformation3<Data<QUuid, FullData>, true> aZ;
  }
}
