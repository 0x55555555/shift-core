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
  Eks::Optional<ResolveResult> res(resultOpt);

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
  Eks::Optional<ResolveResult> res(resultOpt);

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

class ExternalUuidPointer::Traits : public detail::PropertyBaseTraits
  {
public:
  static void saveProperty(const Property *p, Saver &s )
    {
    detail::PropertyBaseTraits::saveProperty(p, s, false);

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

  static Property *loadProperty(PropertyContainer *parent, Loader &l)
    {
    Property *p = detail::PropertyBaseTraits::loadProperty(parent, l);

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
  };

S_IMPLEMENT_PROPERTY(ExternalUuidPointer, Shift)

void ExternalUuidPointer::createTypeInformation(PropertyInformationTyped<ExternalUuidPointer> *,
                                            const PropertyInformationCreateData &)
  {
  }

void ExternalUuidPointer::setPointed(const UuidEntity *entity)
  {
  _id = entity->_uuid();
  setInput(entity);
  }

S_IMPLEMENT_PROPERTY(UuidEntity, Shift)

void UuidEntity::createTypeInformation(PropertyInformationTyped<UuidEntity> *info,
                                            const PropertyInformationCreateData &data)
  {
  if(data.registerAttributes)
    {
    info->add(data, &UuidEntity::_uuid, "uuid");
    }
  }

}
