#include "shift/Properties/sset.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"

namespace Shift
{

S_IMPLEMENT_PROPERTY(Set, Shift)

void Set::createTypeInformation(PropertyInformationTyped<Set> *,
                                           const PropertyInformationCreateData &)
  {
  }

void Set::remove(Attribute *prop)
  {
  Container::removeAttribute(prop);
  }

}
