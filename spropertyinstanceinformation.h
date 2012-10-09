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
  XProperty(xsize, index, setIndex);
  XROProperty(Mode, mode);
  XROProperty(bool, isDynamic);

public:
  SPropertyInstanceInformation(bool dynamic);
  static SPropertyInstanceInformation *allocate(xsize size);
  static void destroy(SPropertyInstanceInformation *);

  void setMode(Mode);
  void setModeString(const QString &);
  bool isDefaultMode() const;
  const QString &modeString() const;

  virtual void setDefaultValue(const QString &);

  virtual void initiateProperty(SProperty *propertyToInitiate) const;

  const SPropertyInstanceInformation *resolvePath(const QString &) const;

  void initiate(const SPropertyInformation *info,
                const QString &name,
                xsize index,
                xsize s);

  const SEmbeddedPropertyInstanceInformation* embeddedInfo() const;
  const SDynamicPropertyInstanceInformation* dynamicInfo() const;

  X_ALIGNED_OPERATOR_NEW
  };

class SEmbeddedPropertyInstanceInformation : public SPropertyInstanceInformation
  {
  // Static Instance Members
  XProperty(SPropertyInformation *, holdingTypeInformation, setHoldingTypeInformation);
  XProperty(xsize, location, setLocation);
  XROProperty(ComputeFunction, compute);
  XROProperty(xsize *, affects);
  XProperty(bool, isExtraClassMember, setIsExtraClassMember);
  XROProperty(xptrdiff, defaultInput);
  XPropertyMember(SEmbeddedPropertyInstanceInformation *, nextSibling);

public:
  SEmbeddedPropertyInstanceInformation() : SPropertyInstanceInformation(false)
    {
    }

  SEmbeddedPropertyInstanceInformation *nextSibling() { return _nextSibling; }
  const SEmbeddedPropertyInstanceInformation *nextSibling() const { return _nextSibling; }

  template <typename T> const SEmbeddedPropertyInstanceInformation *nextSibling() const;

  SProperty *locateProperty(SPropertyContainer *parent) const;
  const SProperty *locateProperty(const SPropertyContainer *parent) const;

  const SPropertyContainer *locateConstParent(const SProperty *prop) const;
  SPropertyContainer *locateParent(SProperty *prop) const;

  void setCompute(ComputeFunction fn);
  bool isComputed() const { return _compute != 0; }

  void addAffects(const SPropertyInstanceInformation *info);
  void setAffects(const SPropertyInstanceInformation *info);
  void setAffects(const SPropertyInstanceInformation **info, xsize size);
  void setAffects(xsize *affects);
  bool affectsSiblings() const { return _affects != 0; }

  void setDefaultInput(const SPropertyInstanceInformation *info);
  };

class SDynamicPropertyInstanceInformation : public SPropertyInstanceInformation
  {
  // Dynamic Instance
  XProperty(SPropertyContainer *, dynamicParent, setDynamicParent)
  XProperty(SProperty *, dynamicNextSibling, setDynamicNextSibling)

public:
  SDynamicPropertyInstanceInformation() : SPropertyInstanceInformation(true)
    {
    }
  };

template <typename T> const SEmbeddedPropertyInstanceInformation *SEmbeddedPropertyInstanceInformation::nextSibling() const
  {
  const SPropertyInformation *info = T::staticTypeInformation();
  const SPropertyInstanceInformation *next = _nextSibling;
  while(next)
    {
    const SPropertyInformation *nextInfo = next->childInformation();
    if(nextInfo->inheritsFromType(info))
      {
      return next;
      }
    next = next->nextSibling();
    }
  return 0;
  }

const SEmbeddedPropertyInstanceInformation* SPropertyInstanceInformation::embeddedInfo() const
  {
  xAssert(!isDynamic());
  return static_cast<const SEmbeddedPropertyInstanceInformation*>(this);
  }

const SDynamicPropertyInstanceInformation* SPropertyInstanceInformation::dynamicInfo() const
  {
  xAssert(isDynamic());
  return static_cast<const SDynamicPropertyInstanceInformation*>(this);
  }

template <typename T> const SPropertyInstanceInformation *SPropertyInformation::firstChild() const
  {
  const SPropertyInformation *info = T::staticTypeInformation();
  const SPropertyInstanceInformation *first = firstChild();
  while(first)
    {
    const SPropertyInformation *firstInfo = first->childInformation();
    if(firstInfo->inheritsFromType(info))
      {
      return first;
      }
    first = first->nextSibling();
    }
  return 0;
  }

#endif // SPROPERTYINSTANCEINFORMATION_H
