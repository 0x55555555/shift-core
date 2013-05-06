#include "shift/Properties/sbasepointerproperties.h"
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

/*const PropertyInformation *Pointer::pointedToType() const
  {
  return pointedToType(typeInformation());
  }

PropertyInformation::DataKey g_pointerTypeKey = PropertyInformation::newDataKey();
const PropertyInformation *Pointer::pointedToType(const PropertyInformation *info)
  {
  xAssert(info);

  QVariant var = info->data()[g_pointerTypeKey];
  if(!var.isValid())
    {
    var = pointedToType(info->parentTypeInformation());
    }
  xAssert(var.isValid());

  const PropertyInformation *pointedTo = var.value<const PropertyInformation*>();
  xAssert(pointedTo);

  return pointedTo;
  }*/

S_IMPLEMENT_TYPED_POINTER_ARRAY_TYPE(PointerArray, Shift)

}
