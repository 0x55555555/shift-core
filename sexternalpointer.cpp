#include "sexternalpointer.h"
#include "spropertyinformationhelpers.h"
#include "XOptionalPointer"

ExternalPointerInstanceInformation::ExternalPointerInstanceInformation()
    : _resolveFunction(0)
  {
  }

S_IMPLEMENT_PROPERTY(ExternalPointer, Shift)

void ExternalPointer::createTypeInformation(SPropertyInformationTyped<ExternalPointer> *,
                                            const SPropertyInformationCreateData &)
  {
  }

SProperty *ExternalPointer::resolve(ResolveResult *resultOpt)
  {
  XOptional<ResolveResult> res(resultOpt);

  SProperty *inp = input();
  if(inp)
    {
    return inp;
    }

  const EmbeddedInstanceInformation *inst = embeddedInstanceInformation();
  EmbeddedInstanceInformation::ResolveExternalPointer resFn = inst->resolveFunction();
  xAssert(res);

  SProperty *prop = resFn(this, inst, resultOpt);
  setInput(prop);

  return prop;
  }

const SProperty *ExternalPointer::resolve(ResolveResult *resultOpt) const
  {
  XOptional<ResolveResult> res(resultOpt);

  const SProperty *inp = input();
  if(inp)
    {
    return inp;
    }

  const EmbeddedInstanceInformation *inst = embeddedInstanceInformation();
  EmbeddedInstanceInformation::ResolveExternalPointer resFn = inst->resolveFunction();
  xAssert(res);

  const SProperty *prop = resFn(this, inst, resultOpt);
  return prop;
  }

S_IMPLEMENT_PROPERTY(ExternalUuidPointer, Shift)

void ExternalUuidPointer::createTypeInformation(SPropertyInformationTyped<ExternalUuidPointer> *,
                                            const SPropertyInformationCreateData &)
  {
  }

void ExternalUuidPointer::setPointed(const SUuidEntity *entity)
  {
  _id = entity->_id;
  setInput(entity);
  }

S_IMPLEMENT_PROPERTY(ExternalPointer, Shift)

void SUuidEntity::createTypeInformation(SPropertyInformationTyped<ExternalUUIDPointer> *,
                                            const SPropertyInformationCreateData &)
  {
  }
