#ifndef SPROPERTY_H
#define SPROPERTY_H

#include "sglobal.h"
#include "schange.h"
#include "spropertymacros.h"
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
class SPropertyDataChange;
class SPropertyConnectionChange;
class SPropertyNameChange;

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

  template <typename T> T *output() const;

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

  SHandler *handler();
  const SHandler *handler() const;

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
    /*if(_flags.hasFlag(ParentHasInput))
      {
      updateParent();
      }*/

    if(_flags.hasFlag(Dirty))
      {
      update();
      }
    }
  void update() const;
  void updateParent() const;

  bool isDynamic() const;

  // find a path from this to that
  QString pathTo(const SProperty *that) const;
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

  typedef SPropertyDataChange DataChange;
  typedef SPropertyConnectionChange ConnectionChange;
  typedef SPropertyNameChange NameChange;

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

  const InstanceInformation *_instanceInfo;

  enum Flags
    {
    Dirty = 1,
    ParentHasInput = 2,
    ParentHasOutput = 4,
    PreGetting = 8
    };
  XFlags<Flags, xuint8> _flags;

#ifdef S_CENTRAL_CHANGE_HANDLER
  SHandler *_handler;
#endif

#ifdef S_PROPERTY_USER_DATA
  UserData *_userData;
#endif

  friend class SEntity;
  friend class SDatabase;
  friend class SPropertyContainer;
  friend class SPropertyInstanceInformation;
  friend class SProcessManager;
  friend class SPropertyDataChange;
  friend class SPropertyConnectionChange;
  friend class SPropertyNameChange;
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

template <typename T> inline T *SProperty::output() const
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

#endif // SPROPERTY_H
