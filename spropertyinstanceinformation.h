#ifndef SPROPERTYINSTANCEINFORMATION_H
#define SPROPERTYINSTANCEINFORMATION_H

#include "QString"
#include "sglobal.h"

class SProperty;
class SPropertyContainer;
class SPropertyInformation;
class SPropertyInstanceInformation;
class SEmbeddedPropertyInstanceInformation;
class SDynamicPropertyInstanceInformation;

class SPropertyInstanceInformationInitialiser
  {
public:
  virtual void initialise(SPropertyInstanceInformation *) = 0;
  };

// Child information
class SHIFT_EXPORT SPropertyInstanceInformation
  {
public:
  typedef void (*ComputeFunction)( const SPropertyInstanceInformation *, SPropertyContainer * );

  enum Mode
    {
    Internal,
    InputOutput,
    InternalInput,
    Input,
    Output,
    Computed,
    InternalComputed,
    UserSettable,

    NumberOfModes,

    Default = InputOutput
    };

XProperties:
  XProperty(const SPropertyInformation *, childInformation, setChildInformation);
  XRefProperty(QString, name);
  XProperty(xuint16, index, setIndex);
  XPropertyMember(xuint8, mode);
  XROProperty(xuint8, isDynamic);

public:
  SPropertyInstanceInformation(bool dynamic);
  static void destroy(SPropertyInstanceInformation *);

  void setInvalidIndex();

  Mode mode() const;
  void setMode(Mode);
  void setModeString(const QString &);
  bool isDefaultMode() const;
  const QString &modeString() const;

  const SEmbeddedPropertyInstanceInformation* embeddedInfo() const;
  const SDynamicPropertyInstanceInformation* dynamicInfo() const;

  X_ALIGNED_OPERATOR_NEW
  };

class SHIFT_EXPORT SEmbeddedPropertyInstanceInformation : public SPropertyInstanceInformation
  {
  // Static Instance Members
  XProperty(SPropertyInformation *, holdingTypeInformation, setHoldingTypeInformation);
  XProperty(xuint16, location, setLocation);
  XROProperty(xint16, defaultInput);

  XROProperty(ComputeFunction, compute);
  XROProperty(xsize *, affects);

  XPropertyMember(SEmbeddedPropertyInstanceInformation *, nextSibling);

  XProperty(bool, isExtraClassMember, setIsExtraClassMember);

public:
  SEmbeddedPropertyInstanceInformation();

  static SEmbeddedPropertyInstanceInformation *allocate(xsize size);

  void setMode(Mode);

  const SEmbeddedPropertyInstanceInformation *resolvePath(const QString &) const;

  SEmbeddedPropertyInstanceInformation *nextSibling() { return _nextSibling; }
  const SEmbeddedPropertyInstanceInformation *nextSibling() const { return _nextSibling; }

  template <typename T> const SEmbeddedPropertyInstanceInformation *nextSibling() const;

  virtual void setDefaultValue(const QString &);
  virtual void initiateProperty(SProperty *propertyToInitiate) const;

  SProperty *locateProperty(SPropertyContainer *parent) const;
  const SProperty *locateProperty(const SPropertyContainer *parent) const;

  const SPropertyContainer *locateConstParent(const SProperty *prop) const;
  SPropertyContainer *locateParent(SProperty *prop) const;

  void setCompute(ComputeFunction fn);
  bool isComputed() const { return _compute != 0; }

  void addAffects(const SEmbeddedPropertyInstanceInformation *info);
  void setAffects(const SEmbeddedPropertyInstanceInformation *info);
  void setAffects(const SEmbeddedPropertyInstanceInformation **info, xsize size);
  void setAffects(xsize *affects);
  bool affectsSiblings() const { return _affects != 0; }

  void initiate(const SPropertyInformation *info,
                const QString &name,
                xsize index,
                xsize s);

  void setDefaultInput(const SEmbeddedPropertyInstanceInformation *info);
  };

class SHIFT_EXPORT SDynamicPropertyInstanceInformation : public SPropertyInstanceInformation
  {
  // Dynamic Instance
  XProperty(SPropertyContainer *, parent, setParent)
  XProperty(SProperty *, nextSibling, setNextSibling)

public:
  SDynamicPropertyInstanceInformation();
  };

inline const SEmbeddedPropertyInstanceInformation* SPropertyInstanceInformation::embeddedInfo() const
  {
  xAssert(!isDynamic());
  return static_cast<const SEmbeddedPropertyInstanceInformation*>(this);
  }

inline const SDynamicPropertyInstanceInformation* SPropertyInstanceInformation::dynamicInfo() const
  {
  xAssert(isDynamic());
  return static_cast<const SDynamicPropertyInstanceInformation*>(this);
  }

#endif // SPROPERTYINSTANCEINFORMATION_H
