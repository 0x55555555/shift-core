#ifndef SATTRIBUTE_H
#define SATTRIBUTE_H

#include "shift/Properties/spropertymacros.h"
#include "shift/Utilities/spropertyname.h"

namespace Eks
{
class TemporaryAllocatorCore;
}

namespace Shift
{

class Module;
class Container;
class PropertyInstanceInformation;
class EmbeddedPropertyInstanceInformation;
class DynamicPropertyInstanceInformation;
class PropertyInformation;
class Handler;
class Database;
class InterfaceBase;
class PropertyDataChange;
class PropertyNameChange;
template <typename T> class PropertyInformationTyped;
class PropertyInformationCreateData;

namespace detail
{
class PropertyBaseTraits;
}

class SHIFT_EXPORT Attribute
  {
public:
  typedef PropertyInstanceInformation BaseInstanceInformation;
  typedef EmbeddedPropertyInstanceInformation EmbeddedInstanceInformation;
  typedef DynamicPropertyInstanceInformation DynamicInstanceInformation;
  typedef detail::PropertyBaseTraits Traits;

  S_PROPERTY_ROOT(Attribute)

  enum
    {
    StaticChildMode = NoChildren,
    DynamicChildMode = NoChildren
    };

public:
  Attribute();

#ifdef S_PROPERTY_USER_DATA
  ~Attribute();
#endif

  void assign(const Attribute *propToAssign);

  // get the parent entity for this attribute
  // or if this attribute is an entity, get it.
  const Entity *entity() const;
  Entity *entity();

  void setParent(Container *parent);
  const Container *parent() const;
  Container *parent();
  const Container *embeddedParent() const;
  Container *embeddedParent();

  Handler *handler();
  const Handler *handler() const;

  Database *database();
  const Database *database() const;
  void beginBlock();
  void endBlock(bool cancel);

  inline bool isDynamic() const;

  bool equals(const Attribute *) const;

  bool inheritsFromType(const PropertyInformation *type) const;
  template <typename T> bool inheritsFromType() const { return inheritsFromType(T::staticTypeInformation()); }

  inline const PropertyInformation *typeInformation() const;
  inline const BaseInstanceInformation *baseInstanceInformation() const;
  inline const EmbeddedInstanceInformation *embeddedBaseInstanceInformation() const;
  inline const DynamicInstanceInformation *dynamicBaseInstanceInformation() const;

  // find a path from this to that
  Eks::String pathTo(const Attribute *that, Eks::AllocatorBase* allocator) const;
  Eks::String path(Eks::AllocatorBase* allocator) const;
  Eks::String path(const Attribute *from, Eks::AllocatorBase* allocator) const;

  bool isDescendedFrom(const Attribute *ent) const;
  Attribute *resolvePath(const Eks::String &path);
  const Attribute *resolvePath(const Eks::String &path) const;

  const Eks::String &mode() const;

  // set only works for dynamic properties
  void setName(const NameArg &);
  const Name &name() const;
  // unique identifier for parent, possibly name, possibly a stringified index
  Name identifier() const;
  // name valid for entry into paths.
  Name escapedIdentifier() const;

  template <typename T>T *uncheckedCastTo()
    {
    xAssert(castTo<T>());
    return static_cast<T *>(this);
    }

  template <typename T>const T *uncheckedCastTo() const
    {
    xAssert(castTo<T>());
    return static_cast<const T *>(this);
    }

  template <typename T>T *castTo()
    {
    if(inheritsFromType(T::staticTypeInformation()))
      {
      return static_cast<T *>(this);
      }
    return 0;
    }
  template <typename T>const T *castTo() const
    {
    if(inheritsFromType(T::staticTypeInformation()))
      {
      return static_cast<const T *>(this);
      }
    return 0;
    }

  InterfaceBase *findInterface(xuint32 typeId);
  const InterfaceBase *findInterface(xuint32 typeId) const;

  template <typename T> T *findInterface()
    {
    return static_cast<T *>(findInterface(T::InterfaceTypeId));
    }

  template <typename T> const T *findInterface() const
    {
    return static_cast<const T *>(findInterface(T::InterfaceTypeId));
    }

#ifdef S_PROPERTY_USER_DATA
  class UserData
    {
    S_USER_DATA_TYPE(Invalid);
  XProperties:
    XROProperty(UserData *, next);
  public:
    // bool indicates whether the caller should delete the UserData on
    virtual bool onPropertyDelete(Property *) { return false; }
  private:
    friend class Property;
    };

  UserData *firstUserData() { return _userData; }
  void addUserData(UserData *userData);
  void removeUserData(UserData *userData);
#endif

  typedef PropertyDataChange DataChange;
  typedef PropertyNameChange NameChange;

  Eks::TemporaryAllocatorCore *temporaryAllocator() const;
  Eks::AllocatorBase *persistentBlockAllocator() const;
  Eks::AllocatorBase *generalPurposeAllocator() const;

private:
  X_DISABLE_COPY(Attribute);

  // set only works for dynamic properties
  void forceSetName(const Name &);

  void internalSetName(const NameArg &name);

  const BaseInstanceInformation *_instanceInfo;

#ifdef S_PROPERTY_USER_DATA
  UserData *_userData;
#endif

  friend class Entity;
  friend class Database;
  friend class Container;
  friend class PropertyInstanceInformation;
  friend class PropertyDataChange;
  friend class PropertyNameChange;
  friend class detail::PropertyBaseTraits;
  };

#define S_PROPERTY_INTERFACE(name)

#define S_PROPERTY_ABSTRACT_INTERFACE(name)
}

#endif // SATTRIBUTE_H
