#include "shift/QtExtensions/sqtproperties.h"

#if X_QT_INTEROP

#include "shift/Properties/sdata.inl"

namespace Shift
{

IMPLEMENT_POD_SHIFT_PROPERTY(QUuid, FullData)

namespace detail
{

void assignTo(const Attribute *f, Data<QUuid> *to)
  {
  const Data<QUuid> *sProp = f->castTo<Data<QUuid>>();
  if(sProp)
    {
    to->assign(sProp->value());
    return;
    }
  }

void detail::UuidPropertyInstanceInformation::initiateAttribute(
    Attribute *propertyToInitiate,
    AttributeInitialiserHelper *helper) const
  {
  Property::EmbeddedInstanceInformation::initiateAttribute(propertyToInitiate, helper);

  NoUpdateBlock b(propertyToInitiate);

  auto obj = propertyToInitiate->uncheckedCastTo<Data<QUuid>>();
  obj->computeLock() = QUuid::createUuid();
  }

}

}

#endif
