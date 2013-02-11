#include "shift/Properties/spropertyarray.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"

namespace Shift
{

S_IMPLEMENT_PROPERTY(PropertyArray, Shift)

void PropertyArray::createTypeInformation(PropertyInformationTyped<PropertyArray> *info,
                                           const PropertyInformationCreateData &data)
  {
  if(data.registerInterfaces)
    {
    auto* api = info->apiInterface();

    typedef XScript::MethodToInCa<PropertyArray,
        Property *(const PropertyInformation *, xuint8, const PropertyNameArg &),
        &PropertyArray::add> AddType;

    static XScript::ClassDef<0,0,1> cls = {
      {
        api->method<AddType>("add")
      }
    };

    api->buildInterface(cls);
    }
  }

void PropertyArray::remove(Property *prop)
  {
  PropertyContainer::removeProperty(prop);
  }

}
