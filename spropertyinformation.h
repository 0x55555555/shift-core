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

namespace XScript
{
class InterfaceBase;
template <typename T> class Interface;
}

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
  CreateInstanceInformationFunction createEmbeddedInstanceInformation;
  CreateInstanceInformationFunction createDynamicInstanceInformation;
  DestroyInstanceInformationFunction destroyEmbeddedInstanceInformation;
  DestroyInstanceInformationFunction destroyDynamicInstanceInformation;
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

  XROProperty(SEmbeddedPropertyInstanceInformation *, firstChild);
  XROProperty(SEmbeddedPropertyInstanceInformation *, lastChild);
  XProperty(xsize, size, setSize);
  XProperty(xsize, dynamicInstanceInformationSize, setDynamicInstanceInformationSize);
  XProperty(xsize, embeddedInstanceInformationSize, setEmbeddedInstanceInformationSize);

  XRORefProperty(DataHash, data);

  XProperty(xsize, instances, setInstances);

  XProperty(SEmbeddedPropertyInstanceInformation *, extendedParent, setExtendedParent);

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
  SEmbeddedPropertyInstanceInformation *child(xsize location);
  const SEmbeddedPropertyInstanceInformation *child(xsize location) const;

  SEmbeddedPropertyInstanceInformation *childFromName(const QString &);
  const SEmbeddedPropertyInstanceInformation *childFromName(const QString &) const;

  // find the sproperty information that will be allocated dynamically (ie has no static parent)
  // offset is the offset in bytes back from this base to the allocated base.
  SPropertyInformation *findAllocatableBase(xsize &offset);
  const SPropertyInformation *findAllocatableBase(xsize &offset) const;

  void setData(DataKey, const QVariant &);

  // size of the property type, and its instance information
  xsize dynamicSize() const { return size() + dynamicInstanceInformationSize() + X_ALIGN_BYTE_COUNT; }

  SEmbeddedPropertyInstanceInformation *add(const SPropertyInformation *newChildType,
                                    xsize location,
                                    const QString &name,
                                    bool notClassMember);

  SEmbeddedPropertyInstanceInformation *add(const SPropertyInformation *newChildType, const QString &name);

  const SInterfaceBaseFactory *interfaceFactory(xuint32 type) const;
  SPropertyInformation *extendContainedProperty(SEmbeddedPropertyInstanceInformation *inst);

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

  template <typename T> const SEmbeddedPropertyInstanceInformation *firstChild() const;

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

  Walker<SPropertyInformation, SEmbeddedPropertyInstanceInformation> childWalker()
    {
    return Walker<SPropertyInformation, SEmbeddedPropertyInstanceInformation>(this);
    }
  Walker<const SPropertyInformation, const SEmbeddedPropertyInstanceInformation> childWalker() const
    {
    return Walker<const SPropertyInformation, const SEmbeddedPropertyInstanceInformation>(this);
    }

private:
  X_DISABLE_COPY(SPropertyInformation);

  mutable InterfaceHash _interfaceFactories;

  void reference() const;
  void dereference() const;

  friend class SProperty;
  friend class SDatabase;
};

Q_DECLARE_METATYPE(const SPropertyInformation*);

#endif // SPROPERTYINFORMATION_H
