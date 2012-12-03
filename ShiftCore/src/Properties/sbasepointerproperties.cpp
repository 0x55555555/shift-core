#include "shift/Properties/sbasepointerproperties.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"

namespace Shift
{

S_IMPLEMENT_PROPERTY(Pointer, Shift)

void Pointer::createTypeInformation(PropertyInformationTyped<Pointer> *info,
                                    const PropertyInformationCreateData &data)
  {
  if(data.registerInterfaces)
    {
    assignPointerInformation(info, Property::staticTypeInformation());
    }
  }

void Pointer::setPointed(const Property *prop)
  {
  if(hasInput())
    {
    input()->disconnect(this);
    }

  if(prop)
    {
    prop->connect(this);
    }
  }

PropertyInformation::DataKey g_pointerTypeKey = PropertyInformation::newDataKey();
const PropertyInformation *Pointer::pointedToType() const
  {
  return pointedToType(typeInformation());
  }

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
  }

void Pointer::assignPointerInformation(PropertyInformation *newInfo, const PropertyInformation *pointedToInfo)
  {
  newInfo->setData(g_pointerTypeKey, QVariant::fromValue(pointedToInfo));
  }

S_IMPLEMENT_TYPED_POINTER_ARRAY_TYPE(PointerArray, Shift)

}
