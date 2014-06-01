#ifndef SQTPROPERTIES_H
#define SQTPROPERTIES_H

#include "shift/sglobal.h"

#if X_QT_INTEROP

#include "QtCore/QByteArray"
#include "QtCore/QUuid"

namespace Shift
{

SHIFT_EXPORT void assignTo(const Attribute *f, Data<QUuid> *to);

template <> class SHIFT_EXPORT TypedSerialisationValue<QUuid> :  public SerialisationValue
  {
public:
  TypedSerialisationValue(const QUuid *t);

  bool hasUtf8() const X_OVERRIDE { return true; }
  bool hasBinary() const X_OVERRIDE { return false; }

  Eks::String asUtf8(Eks::AllocatorBase *a) const;

private:
  const QUuid *_val;
  };

namespace detail
{
class SHIFT_EXPORT UuidPropertyInstanceInformation : public EmbeddedPropertyInstanceInformation
  {
  typedef QUuid PODType;

public:
  UuidPropertyInstanceInformation()
    {
    }
  void initiateAttribute(Attribute *propertyToInitiate, AttributeInitialiserHelper *) const X_OVERRIDE;

  QUuid defaultValue() const
    {
    return QUuid();
    }
  };

template<> class DataEmbeddedInstanceInformation<Data<QUuid, AttributeData>>
    : public UuidPropertyInstanceInformation
  {
public:
  DataEmbeddedInstanceInformation()
    {
    }
  };

template<> class DataEmbeddedInstanceInformation<Data<QUuid, FullData>>
    : public UuidPropertyInstanceInformation
  {
public:
  DataEmbeddedInstanceInformation()
    {
    }
  };
}

S_DECLARE_METATYPE(Shift::Data<QUuid>, "Uuid");


}

#endif

#endif // SQTPROPERTIES_H
