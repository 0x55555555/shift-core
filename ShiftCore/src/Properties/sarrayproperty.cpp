#include "shift/Properties/sarrayproperty.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/Changes/shandler.inl"

namespace Shift
{

S_IMPLEMENT_PROPERTY(FloatArrayProperty, Shift)

void FloatArrayProperty::createTypeInformation(PropertyInformationTyped<FloatArrayProperty> *,
                                                const PropertyInformationCreateData &)
  {
  }

S_IMPLEMENT_PROPERTY(UIntArrayProperty, Shift)

void UIntArrayProperty::createTypeInformation(PropertyInformationTyped<UIntArrayProperty> *,
                                               const PropertyInformationCreateData &)
  {
  }

S_IMPLEMENT_PROPERTY(Vector2ArrayProperty, Shift)

void Vector2ArrayProperty::createTypeInformation(PropertyInformationTyped<Vector2ArrayProperty> *,
                                                  const PropertyInformationCreateData &)
  {
  }

S_IMPLEMENT_PROPERTY(Vector3ArrayProperty, Shift)

void Vector3ArrayProperty::createTypeInformation(PropertyInformationTyped<Vector3ArrayProperty> *,
                                                  const PropertyInformationCreateData &)
  {
  }

S_IMPLEMENT_PROPERTY(Vector4ArrayProperty, Shift)

void Vector4ArrayProperty::createTypeInformation(PropertyInformationTyped<Vector4ArrayProperty> *,
                                                  const PropertyInformationCreateData &)
  {
  }

}
