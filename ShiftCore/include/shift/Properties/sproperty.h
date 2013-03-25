#ifndef SPROPERTY_H
#define SPROPERTY_H

#include "shift/sglobal.h"
#include "shift/Properties/spropertymacros.h"
#include "shift/Utilities/spropertyname.h"
#include "XFlags"
#include "XInterface.h"

namespace Eks
{
class TemporaryAllocatorCore;
}

namespace Shift
{

class Entity;
class Property;
class PropertyContainer;
class PropertyMetaData;
class Handler;
class Database;
class PropertyInstanceInformation;
class EmbeddedPropertyInstanceInformation;
class DynamicPropertyInstanceInformation;
class PropertyInformation;
template <typename T> class PropertyInformationTyped;
class PropertyInformationCreateData;
class Saver;
class Loader;
class InterfaceBase;
class PropertyDataChange;
class PropertyConnectionChange;
class PropertyNameChange;
class SResourceDescription;

namespace detail
{
class PropertyBaseTraits;
}

class SHIFT_EXPORT Property
  {
public:
  typedef PropertyInstanceInformation BaseInstanceInformation;
  typedef EmbeddedPropertyInstanceInformation EmbeddedInstanceInformation;
  typedef DynamicPropertyInstanceInformation DynamicInstanceInformation;
  typedef detail::PropertyBaseTraits Traits;

  S_PROPERTY_ROOT(Property, 0)

public:
  Property();

#ifdef S_PROPERTY_USER_DATA
  ~Property();
#endif

  void assign(const Property *propToAssign);

  // get the parent entity for this attribute
  // or if this attribute is an entity, get it.
  const Entity *entity() const;
  Entity *entity();

  void setParent(PropertyContainer *parent);
  const PropertyContainer *parent() const;
  PropertyContainer *parent();
  const PropertyContainer *embeddedParent() const;
  PropertyContainer *embeddedParent();

  Property *input() const { return _input; }
  Property *output() const { return _output; }
  Property *nextOutput() const { return _nextOutput; }

  template <typename T> T *output() const;

  // connect this property (driver) to the passed property (driven)
  void connect(Property *) const;
  void setInput(const Property *inp);
  void connect(const Eks::Vector<Property*> &) const;
  void disconnect(Property *) const;
  void disconnect() const;

  bool isDirty() const { return _flags.hasFlag(Dirty); }
  bool isComputed() const;
  bool hasInput() const { return _input; }
  bool hasOutputs() const { return _output; }
  Eks::Vector<const Property *> affects() const;
  Eks::Vector<Property *> affects();

  Handler *handler();
  const Handler *handler() const;

  Database *database();
  const Database *database() const;
  void beginBlock();
  void endBlock(bool cancel);

  bool equals(const Property *) const;

  bool inheritsFromType(const PropertyInformation *type) const;
  template <typename T> bool inheritsFromType() const { return inheritsFromType(T::staticTypeInformation()); }

  inline const PropertyInformation *typeInformation() const;
  inline const BaseInstanceInformation *baseInstanceInformation() const;
  inline const EmbeddedInstanceInformation *embeddedBaseInstanceInformation() const;
  inline const DynamicInstanceInformation *dynamicBaseInstanceInformation() const;

  void postSet();
  void setDependantsDirty();
  void preGet() const
    {
    if(_flags.hasFlag(Dirty))
      {
      update();
      }
    }
  void update() const;
  void updateParent() const;

  inline bool isDynamic() const;

  // find a path from this to that
  Eks::String pathTo(const Property *that) const;
  Eks::String path() const;
  Eks::String path(const Property *from) const;

  bool isDescendedFrom(const Property *ent) const;
  Property *resolvePath(const Eks::String &path);
  const Property *resolvePath(const Eks::String &path) const;

  Eks::String mode() const;

  QVariant value() const;
  void setValue(const QVariant &);
  Eks::String valueAsString() const;

  // set only works for dynamic properties
  void setName(const PropertyNameArg &);
  const PropertyName &name() const;
  // name valid for entry into paths.
  PropertyName escapedName() const;

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

  InterfaceBase *interface(xuint32 typeId);
  const InterfaceBase *interface(xuint32 typeId) const;

  template <typename T> T *interface()
    {
    return static_cast<T *>(interface(T::InterfaceTypeId));
    }

  template <typename T> const T *interface() const
    {
    return static_cast<const T *>(interface(T::InterfaceTypeId));
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
  typedef PropertyConnectionChange ConnectionChange;
  typedef PropertyNameChange NameChange;

  const XScript::InterfaceBase *apiInterface() const;
  static const XScript::InterfaceBase *staticApiInterface();

  Eks::TemporaryAllocatorCore *temporaryAllocator() const;

private:
  X_DISABLE_COPY(Property);

  void setDirty();
  void internalSetName(const PropertyNameArg &name);

  void connectInternal(Property *) const;
  void disconnectInternal(Property *) const;

  Property *_input;
  Property *_output;
  Property *_nextOutput;

  const BaseInstanceInformation *_instanceInfo;

  enum Flags
    {
    Dirty = 1,
    ParentHasInput = 2,
    ParentHasOutput = 4,
    PreGetting = 8
    };
  Eks::Flags<Flags, Eks::detail::Atomic<xuint8> > _flags;

#ifdef S_CENTRAL_CHANGE_HANDLER
  Handler *_handler;
#endif

#ifdef S_PROPERTY_USER_DATA
  UserData *_userData;
#endif

  friend class Entity;
  friend class Database;
  friend class PropertyContainer;
  friend class PropertyInstanceInformation;
  friend class SProcessManager;
  friend class PropertyDataChange;
  friend class PropertyConnectionChange;
  friend class PropertyNameChange;
  friend class detail::PropertyBaseTraits;
  };

template <typename T> inline const XScript::InterfaceBase *findPropertyInterface(const T* prop)
  {
  if(prop)
    {
    return prop->apiInterface();
    }
  return T::staticApiInterface();
  }

#define S_PROPERTY_INTERFACE(name) X_SCRIPTABLE_TYPE_INHERITS(name, Shift::Property) \
  namespace XScript { \
  template <> inline const InterfaceBase *findInterface<name>(const name *p) { \
    return Shift::findPropertyInterface<Shift::Property>(p); } \
  namespace Convert { \
  template <> inline name *castFromBase<name, Shift::Property>(Shift::Property *ptr) { \
    return ptr->castTo<name>(); } } }

#define S_PROPERTY_ABSTRACT_INTERFACE(name) X_SCRIPTABLE_ABSTRACT_TYPE_INHERITS(name, Shift::Property) \
  namespace XScript { \
  template <> inline const InterfaceBase *findInterface<name>(const name *p) { \
    return Shift::findPropertyInterface<Shift::Property>(p); } \
  namespace Convert { \
  template <> inline name *castFromBase<name, Shift::Property>(Shift::Property *ptr) { \
    return ptr->castTo<name>(); } } }

template <typename T> inline T *Property::output() const
  {
  Property *p = output();
  while(p)
    {
    T *t = p->castTo<T>();
    if(t)
      {
      return t;
      }
    p = p->nextOutput();
    }
  return 0;
  }

}

X_SCRIPTABLE_TYPE(Shift::Property)

namespace XScript
{
template <> inline const InterfaceBase *findInterface<Shift::Property>(const Shift::Property* p)
  {
  return Shift::findPropertyInterface<Shift::Property>(p);
  }
}

#endif // SPROPERTY_H
