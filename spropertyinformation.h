#ifndef SPROPERTYINFORMATION_H
#define SPROPERTYINFORMATION_H

#include "sglobal.h"
#include "QString"
#include "sinterface.h"
#include "XProperty"
#include "XHash"
#include "QVariant"

class SProperty;
class SLoader;
class SSaver;
class SPropertyContainer;
class SPropertyInformation;
class SPropertyGroup;

class SPropertyInstanceInformationInitialiser
  {
public:
  virtual void initialise(SPropertyInstanceInformation *) = 0;
  };

namespace XScript
{
class InterfaceBase;
template <typename T> class Interface;
}

// Child information
class SHIFT_EXPORT SPropertyInstanceInformation
  {
public:
  typedef void (*ComputeFunction)( const SPropertyInstanceInformation *, SPropertyContainer * );
  typedef void (*QueueComputeFunction)( const SPropertyInstanceInformation *, const SPropertyContainer *, SProperty **, xsize &numJobs );

  typedef xuint16 DataKey;
  typedef XHash<DataKey, QVariant> DataHash;

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
  XProperty(QueueComputeFunction, queueCompute, setQueueCompute);
  XROProperty(xsize *, affects);
  // this index is internal to this instance information only
  XProperty(xsize, index, setIndex);
  XProperty(bool, extra, setExtra);

  XROProperty(Mode, mode);

  XProperty(bool, dynamic, setDynamic);
  XRORefProperty(DataHash, data);

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

  virtual void setDefaultValue(const QString &) { xAssertFail(); }
  void setDefaultInput(const SPropertyInstanceInformation *info);

  virtual void initiateProperty(SProperty *X_UNUSED(propertyToInitiate)) const;
  static DataKey newDataKey();

  bool isComputed() const { return _compute != 0; }
  bool affectsSiblings() const { return _affects != 0; }

  void setData(DataKey, const QVariant &);

  const SPropertyInstanceInformation *resolvePath(const QString &) const;

  SProperty *locateProperty(SPropertyContainer *parent) const;
  const SProperty *locateProperty(const SPropertyContainer *parent) const;

  const SPropertyContainer *locateConstParent(const SProperty *prop) const;
  SPropertyContainer *locateParent(SProperty *prop) const;

  SPropertyInstanceInformation *nextSibling() { return _nextSibling; }
  const SPropertyInstanceInformation *nextSibling() const { return _nextSibling; }

  template <typename T> const SPropertyInstanceInformation *nextSibling() const;

  X_ALIGNED_OPERATOR_NEW

private:
  void initiate(const SPropertyInformation *info,
                const QString &name,
                xsize index,
                xsize s);

  friend class SProperty;
  friend class SPropertyContainer;
  friend class SPropertyInformation;
  static void defaultQueue(const SPropertyInstanceInformation *, const SPropertyContainer *, SProperty **, xsize &numJobs);
  };

class SPropertyInformationCreateData
  {
public:
  SPropertyInformationCreateData()
    {
    registerAttributes = false;
    registerInterfaces = false;
    }

  bool registerAttributes;
  bool registerInterfaces;
  };

struct SPropertyInformationFunctions
  {
  typedef SProperty *(*CreatePropertyFunction)(void *data);
  typedef void (*CreatePropertyInPlaceFunction)(SProperty *data);
  typedef void (*DestroyPropertyFunction)(SProperty *data);
  typedef void (*PostCreateFunction)(SProperty *data);
  typedef void (*PostSetFunction)(SPropertyContainer *cont, SProperty *data);
  typedef void (*SaveFunction)( const SProperty *, SSaver & );
  typedef SProperty *(*LoadFunction)( SPropertyContainer *, SLoader & );
  typedef void (*AssignFunction)( const SProperty *, SProperty * );
  typedef bool (*SaveQueryFunction)( const SProperty * );
  typedef void (*CreateTypeInformationFunction)(SPropertyInformation *, const SPropertyInformationCreateData &);
  typedef SPropertyInstanceInformation *(*CreateInstanceInformationFunction)(void *data);
  typedef void (*DestroyInstanceInformationFunction)(SPropertyInstanceInformation *data);

  CreateTypeInformationFunction createTypeInformation;
  CreateInstanceInformationFunction createInstanceInformation;
  DestroyInstanceInformationFunction destroyInstanceInformation;
  CreatePropertyFunction createProperty;
  CreatePropertyInPlaceFunction createPropertyInPlace;
  DestroyPropertyFunction destroyProperty;
  SaveFunction save;
  LoadFunction load;
  SaveQueryFunction shouldSave;
  SaveQueryFunction shouldSaveValue;
  AssignFunction assign;
#ifdef S_PROPERTY_POST_CREATE
  PostCreateFunction postCreate;
#endif
  PostSetFunction postChildSet;
  };

class SHIFT_EXPORT SPropertyInformation
  {
public:
  typedef xuint16 DataKey;
  typedef XHash<DataKey, QVariant> DataHash;
  typedef XHash<xuint32, SInterfaceBaseFactory *> InterfaceHash;

  typedef SPropertyInformationFunctions Functions;

XProperties:
  XByRefProperty(SPropertyInformationFunctions, functions, setFunctions);

  XProperty(xsize, propertyDataOffset, setPropertyDataOffset);

  XProperty(xuint32, version, setVersion);

  XByRefProperty(QString, typeName, setTypeName);

  XProperty(const SPropertyInformation *, parentTypeInformation, setParentTypeInformation);

  XROProperty(SPropertyInstanceInformation *, firstChild);
  XROProperty(SPropertyInstanceInformation *, lastChild);
  XProperty(xsize, size, setSize);
  XProperty(xsize, instanceInformationSize, setInstanceInformationSize);

  XRORefProperty(DataHash, data);

  XProperty(xsize, instances, setInstances);

  XProperty(SPropertyInstanceInformation *, extendedParent, setExtendedParent);

  XProperty(XScript::InterfaceBase *, apiInterface, setApiInterface);

  XROProperty(xsize, childCount)

public:
  SPropertyInformation() : _firstChild(0), _lastChild(0), _childCount(0) { }
  static SPropertyInformation *allocate();
  static void destroy(SPropertyInformation *);

  static DataKey newDataKey();

  template <typename T> bool inheritsFromType() const
    {
    return inheritsFromType(T::staticTypeInformation());
    }

  bool inheritsFromType(const SPropertyInformation *type) const;

  // access the properties from offset of member
  SPropertyInstanceInformation *child(xsize location);
  const SPropertyInstanceInformation *child(xsize location) const;

  SPropertyInstanceInformation *childFromName(const QString &);
  const SPropertyInstanceInformation *childFromName(const QString &) const;

  // find the sproperty information that will be allocated dynamically (ie has no static parent)
  // offset is the offset in bytes back from this base to the allocated base.
  SPropertyInformation *findAllocatableBase(xsize &offset);
  const SPropertyInformation *findAllocatableBase(xsize &offset) const;

  void setData(DataKey, const QVariant &);

  // size of the property type, and its instance information
  xsize dynamicSize() const { return size() + instanceInformationSize() + X_ALIGN_BYTE_COUNT; }

  SPropertyInstanceInformation *add(const SPropertyInformation *newChildType,
                                    xsize location,
                                    const QString &name,
                                    bool extra);

  SPropertyInstanceInformation *add(const SPropertyInformation *newChildType, const QString &name);

  const SInterfaceBaseFactory *interfaceFactory(xuint32 type) const;
  SPropertyInformation *extendContainedProperty(SPropertyInstanceInformation *inst);

  template <typename T> void addInterfaceFactory(T *factory)
    {
    addInterfaceFactoryInternal(T::InterfaceType::InterfaceTypeId, factory);
    }
  template <typename T> void addStaticInterface(T *factory)
    {
    addInterfaceFactory(factory);
    }
  void addInterfaceFactoryInternal(xuint32 typeId, SInterfaceBaseFactory *);

#ifdef S_PROPERTY_USER_DATA
  template <typename T> void addAddonInterface() const;
#endif

  template <typename T> static SPropertyInformation *createTypeInformation(const char *,
                                                                                 const SPropertyInformation *parent);
  static SPropertyInformation *createTypeInformationInternal(const char *name,
                                                                   const SPropertyInformation *parent,
                                                                   void (SPropertyInformation *, const char *));

  static SPropertyInformation *derive(const SPropertyInformation *obj);
  static void initiate(SPropertyInformation *info, const SPropertyInformation *from);

  template <typename T> const SPropertyInstanceInformation *firstChild() const
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

  template <typename Cont, typename Member> class Walker
    {
    Cont *_info;
  public:
    class Iterator
      {
      Member *_current;

    public:
      Iterator(Member *c) : _current(c) { }
      Iterator& operator++(int)
        {
        _current = _current->nextSibling();
        return *this;
        }
      Iterator& operator++()
        {
        _current = _current->nextSibling();
        return *this;
        }
      Member *operator*()
        {
        return _current;
        }
      bool operator!=(const Iterator &i) const
        {
        return i._current != _current;
        }
      };
    Walker(Cont *info) : _info(info) { }

    Iterator begin() { return Iterator(_info->firstChild()); }
    Iterator end() { return Iterator(0); }
    };

  Walker<SPropertyInformation, SPropertyInstanceInformation> childWalker() { return Walker<SPropertyInformation, SPropertyInstanceInformation>(this); }
  Walker<const SPropertyInformation, const SPropertyInstanceInformation> childWalker() const { return Walker<const SPropertyInformation, const SPropertyInstanceInformation>(this); }

private:
  X_DISABLE_COPY(SPropertyInformation);

  mutable InterfaceHash _interfaceFactories;

  void reference() const;
  void dereference() const;

  friend class SProperty;
  friend class SDatabase;
};

inline const SPropertyInformation *SProperty::typeInformation() const
  {
  return _instanceInfo->childInformation();
  }

Q_DECLARE_METATYPE(const SPropertyInformation*);


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

#endif // SPROPERTYINFORMATION_H
