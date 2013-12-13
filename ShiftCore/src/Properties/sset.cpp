#include "shift/Properties/sset.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"

namespace Shift
{

S_IMPLEMENT_PROPERTY(Set, Shift)

void Set::createTypeInformation(PropertyInformationTyped<Set> *info,
                                           const PropertyInformationCreateData &data)
  {
  if(data.registerInterfaces)
    {
    auto* api = info->apiInterface();

    typedef XScript::MethodToInCa<Set,
        Attribute *(const PropertyInformation *, xsize, const NameArg &),
        &Set::add> AddType;

    static XScript::ClassDef<0,0,1> cls = {
      {
        api->method<AddType>("add")
      }
    };

    api->buildInterface(cls);
    }
  }

void Set::remove(Attribute *prop)
  {
  Container::removeAttribute(prop);
  }

}
