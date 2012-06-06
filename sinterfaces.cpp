#include "sinterfaces.h"
#include "sbaseproperties.h"
#include "sbasepointerproperties.h"
#include "QColor"
#include "QString"

const QString g_positionName("__position");
const QString g_inputsPositionName("__inputsPosition");
const QString g_outputsPositionName("__outputsPosition");

SBasicPositionInterface::SBasicPositionInterface() : SPropertyPositionInterface(true)
  {
  }

XVector3D SBasicPositionInterface::positionGeneric(const SProperty * p, const QString &name) const
  {
  const SEntity *ent = p->uncheckedCastTo<SEntity>();
  const Vector3DProperty *prop = ent->findChild<Vector3DProperty>(name);
  if(prop)
    {
    return prop->value();
    }
  xAssert(!ent->findChild(name));
  return XVector3D::Zero();
  }

void SBasicPositionInterface::setPositionGeneric(SProperty * p, const XVector3D &val, const QString &name) const
  {
  class Initialiser : public SPropertyInstanceInformationInitialiser
    {
  public:
    void initialise(SPropertyInstanceInformation *inst)
      {
      inst->setMode(SPropertyInstanceInformation::Internal);
      }
    };

  SEntity *ent = p->uncheckedCastTo<SEntity>();
  Vector3DProperty *prop = ent->uncheckedCastTo<SEntity>()->findChild<Vector3DProperty>(name);
  if(!prop)
    {
    Initialiser init;
    prop = ent->addProperty<Vector3DProperty>(name, &init);
    }
  xAssert(prop);

  prop->assign(val);
  }

XVector3D SBasicPositionInterface::position(const SProperty *p) const
  {
  return positionGeneric(p, g_positionName);
  }

void SBasicPositionInterface::setPosition(SProperty *p, const XVector3D &val) const
  {
  setPositionGeneric(p, val, g_positionName);
  }

XVector3D SBasicPositionInterface::inputsPosition(const SProperty *p) const
  {
  return positionGeneric(p, g_inputsPositionName);
  }

void SBasicPositionInterface::setInputsPosition(SProperty *p, const XVector3D &val) const
  {
  setPositionGeneric(p, val, g_inputsPositionName);
  }

XVector3D SBasicPositionInterface::outputsPosition(const SProperty *p) const
  {
  return positionGeneric(p, g_outputsPositionName);
  }

void SBasicPositionInterface::setOutputsPosition(SProperty *p, const XVector3D &val) const
  {
  setPositionGeneric(p, val, g_outputsPositionName);
  }

SBasicColourInterface::SBasicColourInterface() : SPropertyColourInterface(true)
  {
  }

XColour SBasicColourInterface::colour(const SProperty *t) const
  {
  return colour(t->typeInformation());
  }

XColour SBasicColourInterface::colour(const SPropertyInformation *t) const
  {
  if(t->inheritsFromType<Pointer>())
    {
    const SPropertyInformation *pointedTo = Pointer::pointedToType(t);
    return colour(pointedTo);
    }

  xuint32 h = qHash(t->typeName());

  float lightness = (float)(h & 0xFF) / 0xFF;
  float saturation = (float)((h >> 8) & 0xFF) / 0xFF;
  float hue = (float)((h >> 16) & 0xFFF) / 0xFFF; // skip last nibble

  int hueI = hue * 360.0f; // 0.0 -> 360.0
  int satI = ((saturation / 2.0f) + 0.5f) * 255.0f; // 0.5 -> 1.0
  int ligI = ((lightness / 2.0f) + 0.25f) * 255.0f; // 0.25 -> 0.75

  QColor col = QColor::fromHsl(hueI, satI, ligI);

  return XColour(col);
  }
