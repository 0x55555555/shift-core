#include "sarrayproperty.h"
#include "styperegistry.h"
#include "spropertyinformationhelpers.h"

S_IMPLEMENT_PROPERTY(SFloatArrayProperty, Shift)

void SFloatArrayProperty::createTypeInformation(SPropertyInformationTyped<SFloatArrayProperty> *,
                                                const SPropertyInformationCreateData &)
  {
  }

S_IMPLEMENT_PROPERTY(SUIntArrayProperty, Shift)

void SUIntArrayProperty::createTypeInformation(SPropertyInformationTyped<SUIntArrayProperty> *,
                                               const SPropertyInformationCreateData &)
  {
  }

S_IMPLEMENT_PROPERTY(SVector2ArrayProperty, Shift)

void SVector2ArrayProperty::createTypeInformation(SPropertyInformationTyped<SVector2ArrayProperty> *,
                                                  const SPropertyInformationCreateData &)
  {
  }

S_IMPLEMENT_PROPERTY(SVector3ArrayProperty, Shift)

void SVector3ArrayProperty::createTypeInformation(SPropertyInformationTyped<SVector3ArrayProperty> *,
                                                  const SPropertyInformationCreateData &)
  {
  }

S_IMPLEMENT_PROPERTY(SVector4ArrayProperty, Shift)

void SVector4ArrayProperty::createTypeInformation(SPropertyInformationTyped<SVector4ArrayProperty> *,
                                                  const SPropertyInformationCreateData &)
  {
  }
