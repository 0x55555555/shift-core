#ifndef SINTERFACES_H
#define SINTERFACES_H

#include "XMathVector"
#include "sinterface.h"
#include "XColour"

namespace Shift
{

class PropertyVariantInterface : public StaticInterfaceBase
  {
  S_STATIC_INTERFACE_TYPE(PropertyVariantInterface, PropertyVariantInterface);

public:
  PropertyVariantInterface(bool d) : StaticInterfaceBase(d) { }
  virtual Eks::String asString(const Property *) const = 0;
  virtual QVariant asVariant(const Property *) const = 0;
  virtual void setVariant(Property *, const QVariant &) const = 0;
  };

class PropertyPositionInterface : public StaticInterfaceBase
  {
  S_STATIC_INTERFACE_TYPE(PropertyPositionInterface, PropertyPositionInterface);

public:
  PropertyPositionInterface(bool a) : StaticInterfaceBase(a) { }

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
  PropertyConnectionInterface(bool d) : StaticInterfaceBase(d) { }
  virtual void connect(Property *driven, const Property *driver) const = 0;
  };

class SBasicPositionInterface : public PropertyPositionInterface
  {
public:
  SBasicPositionInterface();

  virtual Eks::Vector3D position(const Property *) const;
  virtual void setPosition(Property *, const Eks::Vector3D &) const;

  virtual Eks::Vector3D inputsPosition(const Property *) const;
  virtual void setInputsPosition(Property *, const Eks::Vector3D &) const;

  virtual Eks::Vector3D outputsPosition(const Property *) const;
  virtual void setOutputsPosition(Property *, const Eks::Vector3D &) const;

private:
  virtual Eks::Vector3D positionGeneric(const Property *, const PropertyNameArg &name) const;
  virtual void setPositionGeneric(Property *, const Eks::Vector3D &, const PropertyNameArg &name) const;
  };

class PropertyColourInterface : public StaticInterfaceBase
  {
  S_STATIC_INTERFACE_TYPE(PropertyColourInterface, PropertyColourInterface);

public:
  PropertyColourInterface(bool a) : StaticInterfaceBase(a) { }
  virtual Eks::Colour colour(const Property *) const = 0;
  virtual Eks::Colour colour(const PropertyInformation *) const = 0;
  };

class SBasicColourInterface : public PropertyColourInterface
  {
public:
  SBasicColourInterface();
  virtual Eks::Colour colour(const Property *) const;
  virtual Eks::Colour colour(const PropertyInformation *) const;
  };

}

#endif // SINTERFACES_H
