#include "shift/Properties/sexternalpointer.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/Serialisation/sloader.h"
#include "XOptional"

namespace Shift
{

ExternalPointerInstanceInformation::ExternalPointerInstanceInformation()
    : _resolveFunction(0)
  {
  }

S_IMPLEMENT_PROPERTY(ExternalPointer, Shift)

void ExternalPointer::createTypeInformation(PropertyInformationTyped<ExternalPointer> *,
                                            const PropertyInformationCreateData &)
  {
  }

Property *ExternalPointer::resolve(ResolveResult *resultOpt)
  {
  XOptional<ResolveResult> res(resultOpt);

  Property *inp = input();
  if(inp)
    {
    return inp;
    }

  const EmbeddedInstanceInformation *inst = embeddedInstanceInformation();
  EmbeddedInstanceInformation::ResolveExternalPointer resFn = inst->resolveFunction();
  xAssert(resFn);

  const Property *prop = resFn(this, inst, resultOpt);
  setInput(prop);

  return input();
  }

const Property *ExternalPointer::resolve(ResolveResult *resultOpt) const
  {
  XOptional<ResolveResult> res(resultOpt);

  const Property *inp = input();
  if(inp)
    {
    return inp;
    }

  const EmbeddedInstanceInformation *inst = embeddedInstanceInformation();
  EmbeddedInstanceInformation::ResolveExternalPointer resFn = inst->resolveFunction();
  xAssert(resFn);

  const Property *prop = resFn(this, inst, resultOpt);
  return prop;
  }

S_IMPLEMENT_PROPERTY(ExternalUuidPointer, Shift)

void ExternalUuidPointer::createTypeInformation(PropertyInformationTyped<ExternalUuidPointer> *,
                                            const PropertyInformationCreateData &)
  {
  }

void ExternalUuidPointer::saveProperty(const Property *p, Saver &s )
  {
  Property::saveProperty(p, s, false);

  const ExternalUuidPointer *uuidProp = p->uncheckedCastTo<ExternalUuidPointer>();
  if(s.streamMode() == Saver::Text)
    {
    s.textStream() << uuidProp->_id.toString();
    }
  else
    {
    s.binaryStream() << uuidProp->_id;
    }
  }

Property *ExternalUuidPointer::loadProperty(PropertyContainer *parent, Loader &l)
  {
  Property *p = Property::loadProperty(parent, l);

  ExternalUuidPointer *uuidProp = p->uncheckedCastTo<ExternalUuidPointer>();
  if(l.streamMode() == Saver::Text)
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

void SUuidEntity::createTypeInformation(PropertyInformationTyped<SUuidEntity> *,
                                            const PropertyInformationCreateData &)
  {
  }

SUuidEntity::SUuidEntity()
  {
  _id = QUuid::createUuid();
  }

}
