#include "shift/Properties/sexternalpointer.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/Serialisation/sattributeio.h"
#include "shift/Properties/sdata.inl"
#include "shift/Properties/sbaseproperties.inl"
#include "Utilities/XOptional.h"

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

TypedSerialisationValue<QUuid>::TypedSerialisationValue(const QUuid &t) : _val(t)
  {

  }

Eks::String TypedSerialisationValue<QUuid>::asUtf8(Eks::AllocatorBase *a) const
  {
  Eks::String ret(a);

  ret = _val.toByteArray().constData();
  return ret;
  }

Eks::Vector<xuint8> TypedSerialisationValue<QUuid>::asBinary(Eks::AllocatorBase*) const
  {
  xAssertFail();
  return Eks::Vector<xuint8>();
  }

class ExternalUuidPointer::Traits : public detail::PropertyBaseTraits
  {
public:
  static void save(const Attribute *p, AttributeSaver &s)
    {
    detail::PropertyBaseTraits::save(p, s, false);

    const ExternalUuidPointer *uuidProp = p->uncheckedCastTo<ExternalUuidPointer>();

    s.write(s.valueSymbol(), uuidProp->_id);
    }

  static Attribute *load(Container *parent, Loader &l)
    {
    Attribute *p = detail::PropertyBaseTraits::load(parent, l);

    ExternalUuidPointer *uuidProp = p->uncheckedCastTo<ExternalUuidPointer>();
    if(l.streamMode() == Loader::Text)
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
    auto children = info->createChildrenBlock(data);

    children.add(&UuidEntity::_uuid, "uuid");
    }
  }
}
