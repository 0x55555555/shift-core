#ifndef SPROPERTY_H
#define SPROPERTY_H

#include "sglobal.h"
#include "XObject"
#include "schange.h"
#include "XFlags"
#include "XInterface.h"

class SEntity;
class SProperty;
class SPropertyContainer;
class SPropertyMetaData;
class SHandler;
class SDatabase;
class SPropertyInstanceInformation;
class SPropertyInformation;
template <typename T> class SPropertyInformationTyped;
class SPropertyInformationCreateData;
class SSaver;
class SLoader;
class SInterfaceBase;

#ifdef S_PROPERTY_USER_DATA
#define S_USER_DATA_TYPE(typeId) public: \
  enum { UserDataType = SUserDataTypes::typeId }; \
  virtual xuint32 userDataTypeId() const { return UserDataType; } \
  private:
#endif

#define S_REGISTER_TYPE_FUNCTION(name) \
  public: static void createTypeInformation(SPropertyInformationTyped<name> *info, \
    const SPropertyInformationCreateData &data); \
  static const SPropertyInformation *staticTypeInformation(); \
  static const SPropertyInformation *bootstrapStaticTypeInformation();

#define S_ADD_INSTANCE_INFORMATION(name) const InstanceInformation *instanceInformation() const \
  { return static_cast<const InstanceInformation *>(baseInstanceInformation()); }


#define S_ADD_STATIC_INFO(name, version) \
  public: enum { Version = version, IsAbstract = false };

#define S_ADD_ABSTRACT_STATIC_INFO(name, version) \
  public: enum { Version = version, IsAbstract = true };

#define S_PROPERTY_ROOT(myName, version) \
  public: \
  S_ADD_STATIC_INFO(myName, version); \
  S_ADD_INSTANCE_INFORMATION(myName) \
  typedef void ParentType; \
  S_REGISTER_TYPE_FUNCTION(myName)

#define S_PROPERTY(myName, superName, version) \
  public: \
  S_ADD_STATIC_INFO(myName, version) \
  S_ADD_INSTANCE_INFORMATION(myName) \
  typedef superName ParentType; \
  S_REGISTER_TYPE_FUNCTION(myName)

#define S_ABSTRACT_PROPERTY(myName, superName, version) \
  public: \
  S_ADD_ABSTRACT_STATIC_INFO(myName, version) \
  S_ADD_INSTANCE_INFORMATION(myName) \
  typedef superName ParentType; \
  S_REGISTER_TYPE_FUNCTION(myName)

// its possible we might want to not use a handler globally, and just apply all changes directly.
#if 1
# define SPropertyDoChangeNonLocal(type, ths, ...) {\
  SHandler *hand = ths->handler(); \
  hand->doChange<type>(__VA_ARGS__); \
  }
#endif

#define SPropertyDoChange(type, ...) SPropertyDoChangeNonLocal(type, this, __VA_ARGS__)

class SHIFT_EXPORT SProperty
  {
public:
  typedef SPropertyInstanceInformation InstanceInformation;

  S_PROPERTY_ROOT(SProperty, 0)

public:
  SProperty();
  ~SProperty();

  void assign(const SProperty *propToAssign);

  // get the parent entity for this attribute
  // or if this attribute is an entity, get it.
  const SEntity *entity() const;
  SEntity *entity();

  void setParent(SPropertyContainer *parent);
  const SPropertyContainer *parent() const;
  SPropertyContainer *parent();
  const SPropertyContainer *embeddedParent() const;
  SPropertyContainer *embeddedParent();

  SProperty *input() const {return _input;}
  SProperty *output() const {return _output;}
  SProperty *nextOutput() const {return _nextOutput;}

  template <typename T> T *output() const
    {
    SProperty *p = output();
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

  // connect this property (driver) to the passed property (driven)
  void connect(SProperty *) const;
  void setInput(const SProperty *inp);
  void connect(const QVector<SProperty*> &) const;
  void disconnect(SProperty *) const;
  void disconnect() const;

  bool isDirty() const { return _flags.hasFlag(Dirty); }
  bool isComputed() const;
  bool hasInput() const { return _input; }
  bool hasOutputs() const { return _output; }
  QVector<const SProperty *> affects() const;
  QVector<SProperty *> affects();

  SHandler *handler() { return _handler; }
  const SHandler *handler() const { return _handler; }
  SDatabase *database();
  const SDatabase *database() const;
  void beginBlock();
  void endBlock(bool cancel);

  bool equals(const SProperty *) const;

  bool inheritsFromType(const SPropertyInformation *type) const;
  template <typename T> bool inheritsFromType() const { return inheritsFromType(T::staticTypeInformation()); }

  const SPropertyInformation *typeInformation() const;
  const SPropertyInstanceInformation *baseInstanceInformation() const { xAssert(_instanceInfo); return _instanceInfo; }

  void postSet();
  void setDependantsDirty();
  void preGet() const
    {
    if(_flags.hasFlag(ParentHasInput))
      {
      updateParent();
      }

    if(_flags.hasFlag(Dirty))
      {
      update();
      }
    }
  void update() const;
  void updateParent() const;

  bool isDynamic() const;

  // find a path from this to that
  QString pathTo(const SProperty *that) const
    {
    return that->path(this);
    }

  QString path() const;
  QString path(const SProperty *from) const;

  bool isDescendedFrom(const SProperty *ent) const;
  SProperty *resolvePath(const QString &path);
  const SProperty *resolvePath(const QString &path) const;

  QString mode() const;

  QVariant value() const;
  void setValue(const QVariant &);
  QString valueAsString() const;

  // set only works for dynamic properties
  void setName(const QString &);
  const QString &name() const;
  // name valid for entry into paths.
  QString escapedName() const;

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

  SInterfaceBase *interface(xuint32 typeId);
  const SInterfaceBase *interface(xuint32 typeId) const;

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
    virtual bool onPropertyDelete(SProperty *) { return false; }
  private:
    friend class SProperty;
    };

  UserData *firstUserData() { return _userData; }
  void addUserData(UserData *userData);
  void removeUserData(UserData *userData);
#endif

  class DataChange : public SChange
    {
    S_CHANGE(DataChange, SChange, 53)
  public:
    DataChange(SProperty *p) : _property(p) { }
    SProperty *property() {return _property;}
    const SProperty *property() const {return _property;}
  private:
    SProperty *_property;
    };

  class NameChange : public SChange
    {
    S_CHANGE(NameChange, SChange, 50)
  public:
    NameChange(const QString &b, const QString &a, SProperty *ent)
      : _before(b), _after(a), _property(ent)
      { }
    const QString &before(bool back=false) const
      {
      if(back)
        {
        return _after;
        }
      return _before;
      }
    const QString &after(bool back=false) const
      {
      if(back)
        {
        return _before;
        }
      return _after;
      }
    SProperty *property() {return _property;}
    const SProperty *property() const {return _property;}
  private:
    QString _before;
    QString _after;
    SProperty *_property;
    bool apply();
    bool unApply();
    bool inform(bool back);
    };

  class ConnectionChange : public SChange
    {
    S_CHANGE(ConnectionChange, SChange, 51)
  public:
    enum Mode
      {
      Connect,
      Disconnect
      };

    ConnectionChange(Mode m, SProperty *driver, SProperty *driven)
      : _driver(driver), _driven(driven), _mode(m)
      { }
    SProperty *driver() { return _driver; }
    SProperty *driven() { return _driven; }
    const SProperty *driver() const { return _driver; }
    const SProperty *driven() const { return _driven; }
    Mode mode(bool back=false) const
      {
      if(back)
        {
        return (Mode)(_mode - Disconnect);
        }
      return _mode;
      }

    static void setParentHasInputConnection(SProperty *);
    static void setParentHasOutputConnection(SProperty *);
    static void clearParentHasInputConnection(SProperty *);
    static void clearParentHasOutputConnection(SProperty *);

  private:
    SProperty *_driver;
    SProperty *_driven;
    Mode _mode;
    bool apply();
    bool unApply();
    bool inform(bool back);
    };

  static void assignProperty(const SProperty *, SProperty *);
  static void saveProperty(const SProperty *, SSaver & );
  static SProperty *loadProperty(SPropertyContainer *, SLoader &);
  static void postChildSet(SPropertyContainer *, SProperty *) { xAssertFail(); }

  // should this properties value be saved, for example not when the value
  // is this property's value the default as it is when created.
  static bool shouldSavePropertyValue(const SProperty *);
  // should the property definition itself be saved, note this function must be true if the above is true
  // but the above can be false when this is true.
  static bool shouldSaveProperty(const SProperty *);

  const XInterfaceBase *apiInterface() const;
  static const XInterfaceBase *staticApiInterface();

  X_ALIGNED_OPERATOR_NEW

private:
  X_DISABLE_COPY(SProperty);
  void setDirty();
  void internalSetName(const QString &name);

  void connectInternal(SProperty *) const;
  void disconnectInternal(SProperty *) const;

  SProperty *_input;
  SProperty *_output;
  SProperty *_nextOutput;

  SHandler *_handler;
  const InstanceInformation *_instanceInfo;

  enum Flags
    {
    Dirty = 1,
    ParentHasInput = 2,
    ParentHasOutput = 4,
    PreGetting = 8
    };
  XFlags<Flags, xuint8> _flags;

#ifdef S_PROPERTY_USER_DATA
  UserData *_userData;
#endif

  friend class SEntity;
  friend class SDatabase;
  friend class SPropertyContainer;
  friend class SPropertyInstanceInformation;
  friend class SProcessManager;
  };

template <typename T> inline const XInterfaceBase *findPropertyInterface(const T* prop)
  {
  if(prop)
    {
    return prop->apiInterface();
    }
  return T::staticApiInterface();
  }

#define S_PROPERTY_INTERFACE(name) X_SCRIPTABLE_TYPE_INHERITS(name, SProperty) \
  template <> inline const XInterfaceBase *findInterface<name>(const name *p) { \
    return findPropertyInterface<SProperty>(p); } \
  namespace XScriptConvert { \
  template <> inline name *castFromBase<name, SProperty>(SProperty *ptr) { \
    return ptr->castTo<name>(); } }

#define S_PROPERTY_ABSTRACT_INTERFACE(name) X_SCRIPTABLE_ABSTRACT_TYPE_INHERITS(name, SProperty) \
  template <> inline const XInterfaceBase *findInterface<name>(const name *p) { \
    return findPropertyInterface<SProperty>(p); } \
  namespace XScriptConvert { \
  template <> inline name *castFromBase<name, SProperty>(SProperty *ptr) { \
    return ptr->castTo<name>(); } }

X_SCRIPTABLE_TYPE(SProperty)

template <> inline const XInterfaceBase *findInterface<SProperty>(const SProperty* p)
  {
  return findPropertyInterface<SProperty>(p);
  }

#endif // SPROPERTY_H
