#ifndef SPROPERTYINSTANCEINFORMATION_H
#define SPROPERTYINSTANCEINFORMATION_H

#include "shift/sglobal.h"
#include "shift/Utilities/spropertyname.h"

namespace Shift
{

class PropertyInformationCreateData;
class Property;
class Container;
class PropertyInformation;
class PropertyInstanceInformation;
class EmbeddedPropertyInstanceInformation;
class DynamicPropertyInstanceInformation;
template <typename R, typename T> class PropertyAffectsWalker;

class PropertyInstanceInformationInitialiser
  {
public:
  virtual void initialise(PropertyInstanceInformation*) = 0;
  };

class AttributeInitialiserHelper
  {
public:
  typedef void (*Callback)();
  virtual void onTreeComplete(Callback cb) = 0;
  };

// Child information
class SHIFT_EXPORT PropertyInstanceInformation
  {
public:
  typedef void (*ComputeFunction)( const PropertyInstanceInformation *, Container * );

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
  XRefProperty(Name, name);
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

  static Eks::MemoryResource allocate(
    Eks::AllocatorBase *allocator,
    const Eks::ResourceDescription &fmt);
  static void destroy(Eks::AllocatorBase *allocator, EmbeddedPropertyInstanceInformation *);

  void setMode(Mode);

  const EmbeddedPropertyInstanceInformation *resolvePath(const Eks::String &) const;

  virtual void setDefaultValue(const QString &);
  virtual void initiateAttribute(Attribute *propertyToInitiate, AttributeInitialiserHelper *helper) const;

  Attribute *locate(Container *parent) const;
  const Attribute *locate(const Container *parent) const;

  const Container *locateConstParent(const Attribute *prop) const;
  Container *locateParent(Attribute *prop) const;

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
                const NameArg &name,
                xsize index,
                xsize s);

  void setDefaultInput(const EmbeddedPropertyInstanceInformation *info);

  PropertyAffectsWalker<Property, Container> affectsWalker(Container *c) const;
  PropertyAffectsWalker<const Property, const Container> affectsWalker(const Container *c) const;
  };

class SHIFT_EXPORT DynamicPropertyInstanceInformation : public PropertyInstanceInformation
  {
  // Dynamic Instance
  XProperty(xsize, index, setIndex);
  XProperty(Container *, parent, setParent)
  XProperty(Attribute *, nextSibling, setNextSibling)

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
