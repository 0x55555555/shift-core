#include "spropertyarray.h"
#include "styperegistry.h"
#include "spropertyinformationhelpers.h"

S_IMPLEMENT_PROPERTY(SPropertyArray, Shift)

void SPropertyArray::createTypeInformation(SPropertyInformationTyped<SPropertyArray> *info,
                                           const SPropertyInformationCreateData &data)
  {
  if(data.registerInterfaces)
    {
    auto* api = info->apiInterface();

    typedef XScript::MethodToInCa<SPropertyArray,
        SProperty *(const SPropertyInformation *, xsize index, const QString &),
        &SPropertyArray::add> AddType;

    static XScript::ClassDef<0,0,1> cls = {
      {
        api->method<AddType>("add")
      }
    };

    api->buildInterface(cls);
    }
  }

void SPropertyArray::remove(SProperty *prop)
  {
  SPropertyContainer::removeProperty(prop);
  }
