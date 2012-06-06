#include "sbasepointerproperties.h"
#include "spropertyinformationhelpers.h"

S_IMPLEMENT_PROPERTY(Pointer, Shift)

void Pointer::createTypeInformation(SPropertyInformationTyped<Pointer> *info,
                                    const SPropertyInformationCreateData &data)
  {
  if(data.registerInterfaces)
    {
    assignPointerInformation(info, SProperty::staticTypeInformation());
    }
  }

void Pointer::setPointed(const SProperty *prop)
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

SPropertyInformation::DataKey g_pointerTypeKey = SPropertyInformation::newDataKey();
const SPropertyInformation *Pointer::pointedToType() const
  {
  return pointedToType(typeInformation());
  }

const SPropertyInformation *Pointer::pointedToType(const SPropertyInformation *info)
  {
  xAssert(info);

  QVariant var = info->data()[g_pointerTypeKey];
  if(!var.isValid())
    {
    var = pointedToType(info->parentTypeInformation());
    }
  xAssert(var.isValid());

  const SPropertyInformation *pointedTo = var.value<const SPropertyInformation*>();
  xAssert(pointedTo);

  return pointedTo;
  }

void Pointer::assignPointerInformation(SPropertyInformation *newInfo, const SPropertyInformation *pointedToInfo)
  {
  newInfo->setData(g_pointerTypeKey, QVariant::fromValue(pointedToInfo));
  }

S_IMPLEMENT_TYPED_POINTER_ARRAY_TYPE(PointerArray, Shift)
