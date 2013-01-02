#ifndef SPROPERTYINFORMATION_H
#define SPROPERTYINFORMATION_H

#include "shift/sglobal.h"
#include "shift/TypeInformation/sinterface.h"
#include "QString"
#include "XProperty"
#include "XHash"
#include "QVariant"

namespace XScript
{
class InterfaceBase;
template <typename T> class Interface;
}

namespace Shift
{

class Property;
class Loader;
class Saver;
class PropertyContainer;
class PropertyInformation;
class PropertyGroup;

class PropertyInformationCreateData
  {
public:
  PropertyInformationCreateData(Eks::AllocatorBase *a)
    {
    allocator = a;
    registerAttributes = false;
    registerInterfaces = false;
    }

  bool registerAttributes;
  bool registerInterfaces;
  Eks::AllocatorBase *allocator;
  };

struct PropertyInformationFunctions
  {
  typedef Property *(*CreatePropertyFunction)(void *data);
  typedef void (*CreatePropertyInPlaceFunction)(Property *data);
  typedef void (*DestroyPropertyFunction)(Property *data);
  typedef void (*PostCreateFunction)(Property *data);
  typedef void (*PostSetFunction)(PropertyContainer *cont, Property *data);
  typedef void (*SaveFunction)( const Property *, Saver & );
  typedef Property *(*LoadFunction)( PropertyContainer *, Loader & );
  typedef void (*AssignFunction)( const Property *, Property * );
  typedef bool (*SaveQueryFunction)( const Property * );
  typedef void (*CreateTypeInformationFunction)(PropertyInformation *, const PropertyInformationCreateData &);
  typedef PropertyInstanceInformation *(*CreateInstanceInformationFunction)(void *data);
  typedef void (*DestroyInstanceInformationFunction)(PropertyInstanceInformation *data);

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

class SHIFT_EXPORT PropertyInformation
  {
public:
  typedef xuint16 DataKey;
  typedef Eks::UnorderedMap<DataKey, QVariant> DataHash;
  typedef Eks::UnorderedMap<xuint32, InterfaceBaseFactory *> InterfaceHash;

  typedef PropertyInformationFunctions Functions;

XProperties:
  XByRefProperty(PropertyInformationFunctions, functions, setFunctions);

  XProperty(xsize, propertyDataOffset, setPropertyDataOffset);

  XProperty(xuint32, version, setVersion);

  XByRefProperty(PropertyName, typeName, setTypeName);

  XProperty(const PropertyInformation *, parentTypeInformation, setParentTypeInformation);

  XROProperty(EmbeddedPropertyInstanceInformation **, childData);
  XROProperty(EmbeddedPropertyInstanceInformation **, childEnd);
  XROProperty(EmbeddedPropertyInstanceInformation **, childLimit);

  XProperty(xsize, size, setSize);
  XProperty(xsize, dynamicInstanceInformationSize, setDynamicInstanceInformationSize);
  XProperty(xsize, embeddedInstanceInformationSize, setEmbeddedInstanceInformationSize);

  XRORefProperty(DataHash, data);

  XProperty(xsize, instances, setInstances);

  XProperty(EmbeddedPropertyInstanceInformation *, extendedParent, setExtendedParent);

  XProperty(XScript::InterfaceBase *, apiInterface, setApiInterface);


public:
  PropertyInformation() : _childData(0), _childEnd(0) { }
  static PropertyInformation *allocate(Eks::AllocatorBase *allocator);
  static void destroy(PropertyInformation *, Eks::AllocatorBase *allocator);

  static DataKey newDataKey();

  template <typename T> bool inheritsFromType() const
    {
    return inheritsFromType(T::staticTypeInformation());
    }

  bool inheritsFromType(const PropertyInformation *type) const;

  xsize childCount() const
    {
    return _childEnd - _childData;
    }

  // access the properties from offset of member
  EmbeddedPropertyInstanceInformation *child(xsize location);
  const EmbeddedPropertyInstanceInformation *child(xsize location) const;

  EmbeddedPropertyInstanceInformation *childFromIndex(xsize i)
    {
    return *(_childData + i);
    }

  const EmbeddedPropertyInstanceInformation *childFromIndex(xsize i) const
    {
    return *(_childData + i);
    }

  template <typename T> const EmbeddedPropertyInstanceInformation *firstChild(xsize *i) const;
  template <typename T> const EmbeddedPropertyInstanceInformation *nextChild(xsize *i) const;

  EmbeddedPropertyInstanceInformation *childFromName(const PropertyNameArg &);
  const EmbeddedPropertyInstanceInformation *childFromName(const PropertyNameArg &) const;

  // find the sproperty information that will be allocated dynamically (ie has no static parent)
  // offset is the offset in bytes back from this base to the allocated base.
  PropertyInformation *findAllocatableBase(xsize &offset);
  const PropertyInformation *findAllocatableBase(xsize &offset) const;

  void setData(DataKey, const QVariant &);

  // size of the property type, and its instance information
  xsize dynamicSize() const { return size() + dynamicInstanceInformationSize() + X_ALIGN_BYTE_COUNT; }

  EmbeddedPropertyInstanceInformation *add(const PropertyInformation *newChildType,
                                    xsize location,
                                    const QString &name,
                                    bool notClassMember);

  EmbeddedPropertyInstanceInformation *add(const PropertyInformation *newChildType, const QString &name);

  const InterfaceBaseFactory *interfaceFactory(xuint32 type) const;
  PropertyInformation *extendContainedProperty(EmbeddedPropertyInstanceInformation *inst);

  template <typename T> void addInterfaceFactory(T *factory)
    {
    addInterfaceFactoryInternal(T::InterfaceType::InterfaceTypeId, factory);
    }
  template <typename T> void addStaticInterface(T *factory)
    {
    addInterfaceFactory(factory);
    }
  void addInterfaceFactoryInternal(xuint32 typeId, InterfaceBaseFactory *);

#ifdef S_PROPERTY_USER_DATA
  template <typename T> void addAddonInterface() const;
#endif

  template <typename T> static PropertyInformation *createTypeInformation(const char *,
                                                                                 const PropertyInformation *parent);
  static PropertyInformation *createTypeInformationInternal(const char *name,
                                                                   const PropertyInformation *parent,
                                                                   void (PropertyInformation *, const char *));

  static PropertyInformation *derive(const PropertyInformation *obj);
  static void initiate(PropertyInformation *info, const PropertyInformation *from);

  template <typename Cont, typename Member> class Walker
    {
    Cont *_info;
  public:
    class Iterator
      {
      Cont *_info;
      xsize _index;

    public:
      Iterator(Cont *c, xsize index) : _info(c), _index(index) { }
      Iterator& operator++(int)
        {
        ++_index;
        return *this;
        }
      Iterator& operator++()
        {
        ++_index;
        return *this;
        }
      Member *operator*()
        {
        return _info->childFromIndex(_index);
        }
      bool operator!=(const Iterator &i) const
        {
        return i._index != _index;
        }
      };
    Walker(Cont *info) : _info(info) { }

    Iterator begin() { return Iterator(_info, 0); }
    Iterator end() { return Iterator(_info, _info->childCount()); }
    };

  Walker<PropertyInformation, EmbeddedPropertyInstanceInformation> childWalker()
    {
    return Walker<PropertyInformation, EmbeddedPropertyInstanceInformation>(this);
    }
  Walker<const PropertyInformation, const EmbeddedPropertyInstanceInformation> childWalker() const
    {
    return Walker<const PropertyInformation, const EmbeddedPropertyInstanceInformation>(this);
    }

private:
  X_DISABLE_COPY(PropertyInformation);

  mutable InterfaceHash _interfaceFactories;

  void reference() const;
  void dereference() const;

  friend class Property;
  friend class Database;
};

}

Q_DECLARE_METATYPE(const Shift::PropertyInformation*);

#endif // SPROPERTYINFORMATION_H
