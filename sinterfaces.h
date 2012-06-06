#ifndef SINTERFACES_H
#define SINTERFACES_H

#include "XVector3D"
#include "sinterface.h"
#include "XColour"

class SPropertyVariantInterface : public SStaticInterfaceBase
  {
  S_STATIC_INTERFACE_TYPE(SPropertyVariantInterface, PropertyVariantInterface);

public:
  SPropertyVariantInterface(bool d) : SStaticInterfaceBase(d) { }
  virtual QString asString(const SProperty *) const = 0;
  virtual QVariant asVariant(const SProperty *) const = 0;
  virtual void setVariant(SProperty *, const QVariant &) const = 0;
  };

class SPropertyPositionInterface : public SStaticInterfaceBase
  {
  S_STATIC_INTERFACE_TYPE(SPropertyPositionInterface, PropertyPositionInterface);

public:
  SPropertyPositionInterface(bool a) : SStaticInterfaceBase(a) { }

  virtual XVector3D position(const SProperty *) const = 0;
  virtual void setPosition(SProperty *, const XVector3D &) const = 0;

  virtual XVector3D inputsPosition(const SProperty *) const = 0;
  virtual void setInputsPosition(SProperty *, const XVector3D &) const = 0;

  virtual XVector3D outputsPosition(const SProperty *) const = 0;
  virtual void setOutputsPosition(SProperty *, const XVector3D &) const = 0;
  };

class SPropertyConnectionInterface : public SStaticInterfaceBase
  {
  S_STATIC_INTERFACE_TYPE(SPropertyConnectionInterface, PropertyConnectionInterface);

public:
  SPropertyConnectionInterface(bool d) : SStaticInterfaceBase(d) { }
  virtual void connect(SProperty *driven, const SProperty *driver) const = 0;
  };

class SBasicPositionInterface : public SPropertyPositionInterface
  {
public:
  SBasicPositionInterface();

  virtual XVector3D position(const SProperty *) const;
  virtual void setPosition(SProperty *, const XVector3D &) const;

  virtual XVector3D inputsPosition(const SProperty *) const;
  virtual void setInputsPosition(SProperty *, const XVector3D &) const;

  virtual XVector3D outputsPosition(const SProperty *) const;
  virtual void setOutputsPosition(SProperty *, const XVector3D &) const;

private:
  virtual XVector3D positionGeneric(const SProperty *, const QString &name) const;
  virtual void setPositionGeneric(SProperty *, const XVector3D &, const QString &name) const;
  };

class SPropertyColourInterface : public SStaticInterfaceBase
  {
  S_STATIC_INTERFACE_TYPE(SPropertyColourInterface, PropertyColourInterface);

public:
  SPropertyColourInterface(bool a) : SStaticInterfaceBase(a) { }
  virtual XColour colour(const SProperty *) const = 0;
  virtual XColour colour(const SPropertyInformation *) const = 0;
  };

class SBasicColourInterface : public SPropertyColourInterface
  {
public:
  SBasicColourInterface();
  virtual XColour colour(const SProperty *) const;
  virtual XColour colour(const SPropertyInformation *) const;
  };


#endif // SINTERFACES_H
