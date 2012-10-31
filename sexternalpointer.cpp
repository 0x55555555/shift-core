#include "sexternalpointer.h"
#include "spropertyinformationhelpers.h"
#include "XOptional"
#include "sloader.h"

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
  xAssert(resFn);

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
  xAssert(resFn);

  const SProperty *prop = resFn(this, inst, resultOpt);
  return prop;
  }

S_IMPLEMENT_PROPERTY(ExternalUuidPointer, Shift)

void ExternalUuidPointer::createTypeInformation(SPropertyInformationTyped<ExternalUuidPointer> *,
                                            const SPropertyInformationCreateData &)
  {
  }

void ExternalUuidPointer::saveProperty(const SProperty *p, SSaver &s )
  {
  SProperty::saveProperty(p, s, false);

  const ExternalUuidPointer *uuidProp = p->uncheckedCastTo<ExternalUuidPointer>();
  if(s.streamMode() == SSaver::Text)
    {
    s.textStream() << uuidProp->_id.toString();
    }
  else
    {
    s.binaryStream() << uuidProp->_id;
    }
  }

SProperty *ExternalUuidPointer::loadProperty(SPropertyContainer *parent, SLoader &l)
  {
  SProperty *p = SProperty::loadProperty(parent, l);

  ExternalUuidPointer *uuidProp = p->uncheckedCastTo<ExternalUuidPointer>();
  if(l.streamMode() == SSaver::Text)
    {
    QString str;
    l.textStream() >> str;
    uuidProp->_id = str;
    }
  else
    {
    l.binaryStream() >> uuidProp->_id;
    }

  return p;
  }

void ExternalUuidPointer::setPointed(const SUuidEntity *entity)
  {
  _id = entity->_id;
  setInput(entity);
  }

S_IMPLEMENT_PROPERTY(SUuidEntity, Shift)

void SUuidEntity::createTypeInformation(SPropertyInformationTyped<SUuidEntity> *,
                                            const SPropertyInformationCreateData &)
  {
  }

SUuidEntity::SUuidEntity()
  {
  _id = QUuid::createUuid();
  }
