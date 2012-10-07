#ifndef SPROPERTYINSTANCEINFORMATION_H
#define SPROPERTYINSTANCEINFORMATION_H

#include "QString"
#include "sglobal.h"

class SProperty;
class SPropertyContainer;
class SPropertyInformation;
class SPropertyInstanceInformation;

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
  XProperty(SPropertyInformation *, holdingTypeInformation, setHoldingTypeInformation);
  XRefProperty(QString, name);
  XProperty(xsize, location, setLocation);
  XROProperty(ComputeFunction, compute);
  XProperty(bool, computeLockedToMainThread, setComputeLockedToMainThread);
  //XProperty(QueueComputeFunction, queueCompute, setQueueCompute);
  XROProperty(xsize *, affects);
  // this index is internal to this instance information only
  XProperty(xsize, index, setIndex);
  XProperty(bool, isExtraClassMember, setIsExtraClassMember);

  XROProperty(Mode, mode);

  XProperty(bool, dynamic, setDynamic);
  //XRORefProperty(DataHash, data);

  XROProperty(xptrdiff, defaultInput);

  XPropertyMember(SPropertyInstanceInformation *, nextSibling);

  XProperty(SPropertyContainer *, dynamicParent, setDynamicParent)
  XProperty(SProperty *, dynamicNextSibling, setDynamicNextSibling)

public:
  SPropertyInstanceInformation();
  static SPropertyInstanceInformation *allocate(xsize size);
  static void destroy(SPropertyInstanceInformation *);

  void setMode(Mode);
  void setModeString(const QString &);
  bool isDefaultMode() const;
  const QString &modeString() const;

  void setCompute(ComputeFunction fn);
  void addAffects(const SPropertyInstanceInformation *info);
  void setAffects(const SPropertyInstanceInformation *info);
  void setAffects(const SPropertyInstanceInformation **info, xsize size);
  void setAffects(xsize *affects);

  virtual void setDefaultValue(const QString &);
  void setDefaultInput(const SPropertyInstanceInformation *info);

  virtual void initiateProperty(SProperty *propertyToInitiate) const;

  bool isComputed() const { return _compute != 0; }
  bool affectsSiblings() const { return _affects != 0; }

  const SPropertyInstanceInformation *resolvePath(const QString &) const;

  SProperty *locateProperty(SPropertyContainer *parent) const;
  const SProperty *locateProperty(const SPropertyContainer *parent) const;

  const SPropertyContainer *locateConstParent(const SProperty *prop) const;
  SPropertyContainer *locateParent(SProperty *prop) const;

  SPropertyInstanceInformation *nextSibling() { return _nextSibling; }
  const SPropertyInstanceInformation *nextSibling() const { return _nextSibling; }

  template <typename T> const SPropertyInstanceInformation *nextSibling() const;

  void initiate(const SPropertyInformation *info,
                const QString &name,
                xsize index,
                xsize s);

  X_ALIGNED_OPERATOR_NEW
  };

template <typename T> const SPropertyInstanceInformation *SPropertyInstanceInformation::nextSibling() const
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

#endif // SPROPERTYINSTANCEINFORMATION_H
