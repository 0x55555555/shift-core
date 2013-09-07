#include "shift/Properties/sexternalpointer.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/Serialisation/sloader.h"
#include "shift/Properties/sdata.inl"
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
  static void save(const Attribute *p, Saver &s )
    {
    detail::PropertyBaseTraits::save(p, s, false);

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

  static Attribute *load(Container *parent, Loader &l)
    {
    Attribute *p = detail::PropertyBaseTraits::load(parent, l);

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
  //_id = entity->_uuid();
  setInput(entity);
  }

void thing();

S_IMPLEMENT_PROPERTY(UuidEntity, Shift)

void UuidEntity::createTypeInformation(PropertyInformationTyped<UuidEntity> *info,
                                            const PropertyInformationCreateData &data)
  {
  if(data.registerAttributes)
    {    
    //typedef decltype(UuidEntity::_uuid) UuidType;
    //typedef UuidType::Traits::template TypeTraits<UuidType>::Type TypeTraits;
    //const PropertyInformation *propInfo = UuidType::bootstrapStaticTypeInformation(data.allocator);
    
    thing();

    detail::PODEmbeddedInstanceInformation<Data<QUuid, FullData>, true> aX;
    detail::PODEmbeddedInstanceInformation2<Data<QUuid, FullData>, true> aY;
    detail::PODEmbeddedInstanceInformation3<Data<QUuid, FullData>, true> aZ;

    Data<int, FullData>::EmbeddedInstanceInformation aZ2;
    Data<QUuid, FullData>::EmbeddedInstanceInformation aZ3;

    /*detail::UuidPropertyInstanceInformation a1;
    detail::PODEmbeddedInstanceInformation<Data<QUuid, FullData>, true> a2;
    TypeTraits::StInst *inst = &a2;
    detail::UuidPropertyInstanceInformation *ptr = &a2;
    Data<QUuid> pork;
    inst->initiateAttribute(&pork);
    ptr->initiateAttribute(&pork);
    //a->initiateAttribute(&pork);
*/

    auto children = info->createChildrenBlock(data);
    //children.add(&UuidEntity::_uuid, "uuid");
    }
  }

void detail::UuidPropertyInstanceInformation::initiateAttribute(Attribute *propertyToInitiate) const
  {
  Property::EmbeddedInstanceInformation::initiateAttribute(propertyToInitiate);
  propertyToInitiate->uncheckedCastTo<Data<QUuid>>()->computeLock() = QUuid::createUuid();
  }

}
