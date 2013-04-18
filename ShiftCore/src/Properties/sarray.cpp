#include "shift/Properties/sarray.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"

namespace Shift
{

S_IMPLEMENT_PROPERTY(Array, Shift)

void Array::createTypeInformation(PropertyInformationTyped<Array> *info,
                                           const PropertyInformationCreateData &data)
  {
  if(data.registerInterfaces)
    {
    auto* api = info->apiInterface();

    typedef XScript::MethodToInCa<Array,
        Attribute *(const PropertyInformation *, xsize, const NameArg &),
        &Array::add> AddType;

    static XScript::ClassDef<0,0,1> cls = {
      {
        api->method<AddType>("add")
      }
    };

    api->buildInterface(cls);
    }
  }

void Array::remove(Attribute *prop)
  {
  Container::removeAttribute(prop);
  }

}
