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

    typedef XScript::MethodToInCa<SPropertyArray, SProperty *(const SPropertyInformation *, const QString &), &SPropertyArray::add> FunctionType;
    api->addFunction("add", FunctionType::Arity, FunctionType::Call, FunctionType::CallDart);
    }
  }

void SPropertyArray::remove(SProperty *prop)
  {
  SPropertyContainer::removeProperty(prop);
  }
