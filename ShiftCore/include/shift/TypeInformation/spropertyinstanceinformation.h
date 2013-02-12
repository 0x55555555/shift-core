#ifndef SPROPERTYINSTANCEINFORMATION_H
#define SPROPERTYINSTANCEINFORMATION_H

#include "shift/sglobal.h"
#include "shift/Utilities/spropertyname.h"

namespace Shift
{

class PropertyInformationCreateData;
class Property;
class PropertyContainer;
class PropertyInformation;
class PropertyInstanceInformation;
class EmbeddedPropertyInstanceInformation;
class DynamicPropertyInstanceInformation;

class PropertyInstanceInformationInitialiser
  {
public:
  virtual void initialise(PropertyInstanceInformation*) = 0;
  };

// Child information
class SHIFT_EXPORT PropertyInstanceInformation
  {
public:
  typedef void (*ComputeFunction)( const PropertyInstanceInformation *, PropertyContainer * );

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
  XProperty(const PropertyInformation *, childInformation, setChildInformation);
  XRefProperty(PropertyName, name);
  XPropertyMember(xuint8, mode);
  XROProperty(xuint8, isDynamic);
  XProperty(xuint8, referenceCount, setReferenceCount);

public:
  PropertyInstanceInformation(bool dynamic);
  PropertyInstanceInformation(const PropertyInstanceInformation &);
  static void destroy(Eks::AllocatorBase *allocator, PropertyInstanceInformation *);

  Mode mode() const;
  void setMode(Mode);
  void setModeString(const Eks::String &);
  bool isDefaultMode() const;
  const Eks::String &modeString() const;

  const EmbeddedPropertyInstanceInformation* embeddedInfo() const;
  const DynamicPropertyInstanceInformation* dynamicInfo() const;
  };

class SHIFT_EXPORT EmbeddedPropertyInstanceInformation : public PropertyInstanceInformation
  {
  // Static Instance Members
  XProperty(PropertyInformation *, holdingTypeInformation, setHoldingTypeInformation);
  XProperty(xuint16, location, setLocation);
  XROProperty(xint16, defaultInput);
  XProperty(xuint8, index, setIndex);

  XROProperty(ComputeFunction, compute);
  XROProperty(xsize *, affects);

  XROProperty(bool, affectsOwner);

  XProperty(bool, isExtraClassMember, setIsExtraClassMember);

public:
  EmbeddedPropertyInstanceInformation();
  EmbeddedPropertyInstanceInformation(const EmbeddedPropertyInstanceInformation &);

  static EmbeddedPropertyInstanceInformation *allocate(
    Eks::AllocatorBase *allocator,
    const Eks::ResourceDescription &fmt);

  void setMode(Mode);

  const EmbeddedPropertyInstanceInformation *resolvePath(const Eks::String &) const;

  virtual void setDefaultValue(const QString &);
  virtual void initiateProperty(Property *propertyToInitiate) const;

  Property *locateProperty(PropertyContainer *parent) const;
  const Property *locateProperty(const PropertyContainer *parent) const;

  const PropertyContainer *locateConstParent(const Property *prop) const;
  PropertyContainer *locateParent(Property *prop) const;

  void setCompute(ComputeFunction fn);
  bool isComputed() const { return _compute != 0; }

  //void addAffects(const EmbeddedPropertyInstanceInformation *info);
  void setAffects(
      const PropertyInformationCreateData &data,
      const EmbeddedPropertyInstanceInformation *info);
  void setAffects(
      const PropertyInformationCreateData &data,
      const EmbeddedPropertyInstanceInformation **info,
      xsize size);
  void setAffects(xsize *affects, bool owner);
  bool affectsSiblings() const { return _affects != 0; }

  void initiate(const PropertyInformation *info,
                const PropertyNameArg &name,
                xsize index,
                xsize s);

  void setDefaultInput(const EmbeddedPropertyInstanceInformation *info);
  };

class SHIFT_EXPORT DynamicPropertyInstanceInformation : public PropertyInstanceInformation
  {
  // Dynamic Instance
  XProperty(xsize, index, setIndex);
  XProperty(PropertyContainer *, parent, setParent)
  XProperty(Property *, nextSibling, setNextSibling)

public:
  DynamicPropertyInstanceInformation();
  DynamicPropertyInstanceInformation(const DynamicPropertyInstanceInformation &);

  void setInvalidIndex();
  };

inline const EmbeddedPropertyInstanceInformation* PropertyInstanceInformation::embeddedInfo() const
  {
  xAssert(!isDynamic());
  return static_cast<const EmbeddedPropertyInstanceInformation*>(this);
  }

inline const DynamicPropertyInstanceInformation* PropertyInstanceInformation::dynamicInfo() const
  {
  xAssert(isDynamic());
  return static_cast<const DynamicPropertyInstanceInformation*>(this);
  }

}

#endif // SPROPERTYINSTANCEINFORMATION_H
