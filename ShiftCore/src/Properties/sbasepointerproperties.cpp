#include "shift/Properties/sbasepointerproperties.h"
#include "shift/Properties/scontainer.inl"
#include "shift/TypeInformation/spropertyinformationhelpers.h"

namespace Shift
{

S_IMPLEMENT_PROPERTY(Pointer, Shift)

void Pointer::createTypeInformation(PropertyInformationTyped<Pointer> *,
                                    const PropertyInformationCreateData &)
  {
  }

void Pointer::setPointed(const Property *prop)
  {
  setInput(prop);
  }

S_IMPLEMENT_TYPED_POINTER_ARRAY_TYPE(PointerArray, Shift)

}
