#ifndef SINTERFACES_H
#define SINTERFACES_H

#include "Math/XMathVector.h"
#include "sinterface.h"
#include "Math/XColour.h"
#include "Math/XQuaternion.h"
#include "Containers/XStringBuffer.h"

namespace Shift
{

class PropertyVariantInterface : public StaticInterfaceBase
  {
  S_STATIC_INTERFACE_TYPE(PropertyVariantInterface, PropertyVariantInterface);

public:
  virtual Eks::String asString(const Attribute *) const = 0;
  virtual QVariant asVariant(const Attribute *) const = 0;
  virtual void setVariant(Attribute *, const QVariant &) const = 0;
  };

class PropertyPositionInterface : public StaticInterfaceBase
  {
  S_STATIC_INTERFACE_TYPE(PropertyPositionInterface, PropertyPositionInterface);

public:
  virtual Eks::Vector3D position(const Property *) const = 0;
  virtual void setPosition(Property *, const Eks::Vector3D &) const = 0;

  virtual Eks::Vector3D inputsPosition(const Property *) const = 0;
  virtual void setInputsPosition(Property *, const Eks::Vector3D &) const = 0;

  virtual Eks::Vector3D outputsPosition(const Property *) const = 0;
  virtual void setOutputsPosition(Property *, const Eks::Vector3D &) const = 0;
  };

class PropertyConnectionInterface : public StaticInterfaceBase
  {
  S_STATIC_INTERFACE_TYPE(PropertyConnectionInterface, PropertyConnectionInterface);

public:
  virtual void connect(Property *driven, const Property *driver) const = 0;
  };

class SBasicPositionInterface : public PropertyPositionInterface
  {
public:
  virtual Eks::Vector3D position(const Property *) const;
  virtual void setPosition(Property *, const Eks::Vector3D &) const;

  virtual Eks::Vector3D inputsPosition(const Property *) const;
  virtual void setInputsPosition(Property *, const Eks::Vector3D &) const;

  virtual Eks::Vector3D outputsPosition(const Property *) const;
  virtual void setOutputsPosition(Property *, const Eks::Vector3D &) const;

private:
  virtual Eks::Vector3D positionGeneric(const Property *, const NameArg &name) const;
  virtual void setPositionGeneric(Property *, const Eks::Vector3D &, const NameArg &name) const;
  };

class PropertyColourInterface : public StaticInterfaceBase
  {
  S_STATIC_INTERFACE_TYPE(PropertyColourInterface, PropertyColourInterface);

public:
  virtual Eks::Colour colour(const Property *) const = 0;
  virtual Eks::Colour colour(const PropertyInformation *) const = 0;
  };

class SBasicColourInterface : public PropertyColourInterface
  {
public:
  virtual Eks::Colour colour(const Property *) const;
  virtual Eks::Colour colour(const PropertyInformation *) const;
  };

template <typename PROP, typename POD> class PODPropertyVariantInterface : public PropertyVariantInterface
  {
public:
#if X_QT_INTEROP
  virtual Eks::String asString(const Attribute *p) const
    {
    Eks::String d;
    Eks::String::Buffer buf(&d);
    Eks::String::OStream str(&buf);
    str << p->uncheckedCastTo<PROP>()->value();
    return d;
    }
  virtual QVariant asVariant(const Attribute *p) const
    {
    return QVariant::fromValue<POD>(p->uncheckedCastTo<PROP>()->value());
    }

  virtual void setVariant(Attribute *p, const QVariant &v) const
    {
    p->uncheckedCastTo<PROP>()->assign(v.value<POD>());
    }
#endif
  };

void setupBaseInterfaces();
}

#endif // SINTERFACES_H
