#include "shift/Properties/sarray.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"

namespace Shift
{

S_IMPLEMENT_PROPERTY(Array, Shift)

void Array::createTypeInformation(PropertyInformationTyped<Array> *,
                                           const PropertyInformationCreateData &)
  {
  }

void Array::remove(Attribute *prop)
  {
  Container::removeAttribute(prop);
  }

}
