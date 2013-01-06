#ifndef SPROPERTYTRAITS_H
#define SPROPERTYTRAITS_H

#include "spropertyinformation.h"

namespace Shift
{

class PropertyTraits
  {
public:
  template <typename PropType> static void build(PropertyInformationFunctions &fns)
    {

    // update copy constructor too
    fns.createProperty = PropertyHelper<PropType>::create;
    fns.createPropertyInPlace = PropertyHelper<PropType>::createInPlace;
    fns.destroyProperty = PropertyHelper<PropType>::destroy;
    fns.createEmbeddedInstanceInformation = InstanceInformationHelper<PropType>::createEmbedded;
    fns.createDynamicInstanceInformation = InstanceInformationHelper<PropType>::createDynamic;
    fns.destroyEmbeddedInstanceInformation = InstanceInformationHelper<PropType>::destroyEmbedded;
    fns.destroyDynamicInstanceInformation = InstanceInformationHelper<PropType>::destroyDynamic;

    typedef PropType::Traits Traits;

    fns.save = Traits::saveProperty;
    fns.load = Traits::loadProperty;
    fns.shouldSave = Traits::shouldSaveProperty;
    fns.shouldSaveValue = Traits::shouldSavePropertyValue;
    fns.assign = Traits::assignProperty;

  #ifdef S_PROPERTY_POST_CREATE
    fns.postCreate = 0;
  #endif

    fns.createTypeInformation =
        (PropertyInformationFunctions::CreateTypeInformationFunction)PropType::createTypeInformation;
    }
  };

}
#endif // SPROPERTYTRAITS_H
