#ifndef SPROPERTYINFORMATION_H
#define SPROPERTYINFORMATION_H

#include "shift/sglobal.h"
#include "shift/TypeInformation/sinterface.h"
#include "Utilities/XProperty.h"
#include "Containers/XUnorderedMap.h"

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
class ContainerPropertyInformation;
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
  typedef Attribute *(*CreateFunction)(const Eks::MemoryResource &data);
  typedef void (*CreateInPlaceFunction)(Attribute *data);
  typedef Eks::MemoryResource (*DestroyFunction)(Attribute *data);
  typedef void (*PostCreateFunction)(Attribute *data);
  typedef void (*PostSetFunction)(Attribute *data);
  typedef void (*SaveFunction)( const Attribute *, Saver & );
  typedef Attribute *(*LoadFunction)( Container *, Loader & );
  typedef void (*AssignFunction)( const Attribute *, Attribute * );
  typedef bool (*SaveQueryFunction)( const Attribute * );
  typedef void (*CreateTypeInformationFunction)(PropertyInformation *, const PropertyInformationCreateData &);
  typedef PropertyInstanceInformation *(*CreateInstanceInformationFunction)(const Eks::MemoryResource &data, const PropertyInstanceInformation *copy);
  typedef Eks::MemoryResource (*DestroyInstanceInformationFunction)(PropertyInstanceInformation *data);

  CreateInstanceInformationFunction createEmbeddedInstanceInformation;
  CreateInstanceInformationFunction createDynamicInstanceInformation;
  DestroyInstanceInformationFunction destroyEmbeddedInstanceInformation;
  DestroyInstanceInformationFunction destroyDynamicInstanceInformation;
  CreateFunction create;
  DestroyFunction destroy;
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

  XByRefProperty(Name, typeName, setTypeName);

  XProperty(const PropertyInformation *, parentTypeInformation, setParentTypeInformation);

  XProperty(const EmbeddedPropertyInstanceInformation **, childData, setChildData);
  XProperty(xuint8, childCount, setChildCount);

  XProperty(xuint8, dynamicChildMode, setDynamicChildMode);

  XByRefProperty(
    Eks::ResourceDescription,
    format,
    setFormat);
  XByRefProperty(
    Eks::ResourceDescription,
    dynamicInstanceInformationFormat,
    setDynamicInstanceInformationFormat);
  XByRefProperty(
    Eks::ResourceDescription,
    embeddedInstanceInformationFormat,
    setEmbeddedInstanceInformationFormat);

  XProperty(xsize, instances, setInstances);

  XProperty(EmbeddedPropertyInstanceInformation *, extendedParent, setExtendedParent);

  XProperty(XScript::InterfaceBase *, apiInterface, setApiInterface);


public:
  PropertyInformation() { }
  static PropertyInformation *allocate(Eks::AllocatorBase *allocator);
  static void destroyChildren(PropertyInformation *, Eks::AllocatorBase *allocator);
  static void destroy(PropertyInformation *, Eks::AllocatorBase *allocator);

  template <typename T> bool inheritsFromType() const
    {
    return inheritsFromType(T::staticTypeInformation());
    }

  bool inheritsFromType(const PropertyInformation *type) const;

  // access the properties from offset of member
  const EmbeddedPropertyInstanceInformation *child(xsize location) const;

  const EmbeddedPropertyInstanceInformation *childFromIndex(xsize i) const
    {
    xAssert(i < _childCount);
    return *(_childData + i);
    }

  template <typename T> const EmbeddedPropertyInstanceInformation *firstChild(xsize *i) const;
  template <typename T> const EmbeddedPropertyInstanceInformation *nextChild(xsize *i) const;

  const EmbeddedPropertyInstanceInformation *childFromName(const NameArg& name) const;

  // find the sproperty information that will be allocated dynamically (ie has no static parent)
  // offset is the offset in bytes back from this base to the allocated base.
  PropertyInformation *findAllocatableBase(Eks::RelativeMemoryResource &offset);
  const PropertyInformation *findAllocatableBase(Eks::RelativeMemoryResource &offset) const;

  template <typename T> static PropertyInformation *createTypeInformation(
      const char *name,
      const PropertyInformation *parent);

  static PropertyInformation *createTypeInformationInternal(
      const char *name,
      const PropertyInformation *parent,
      void (Eks::AllocatorBase *, PropertyInformation *, const char *),
      Eks::AllocatorBase *allocator);

  PropertyInformation *extendContainedProperty(
      const PropertyInformationCreateData &data,
      EmbeddedPropertyInstanceInformation *inst);

  static PropertyInformation *derive(
      const PropertyInformation *obj,
      Eks::AllocatorBase *allocator,
      bool addChildren);
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

  PropertyInformationFunctions &functions()
    {
    return _functions;
    }

private:
  X_DISABLE_COPY(PropertyInformation);

  void reference() const;
  void dereference() const;

  friend class Property;
  friend class Database;
};

}

Q_DECLARE_METATYPE(const Shift::PropertyInformation*);

#endif // SPROPERTYINFORMATION_H
