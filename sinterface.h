#ifndef SINTERFACE_H
#define SINTERFACE_H

#include "sproperty.h"

#define S_INTERFACE_FACTORY_TYPE(interfaceType) public: \
  typedef interfaceType InterfaceType; \
  private:

class SInterfaceBase;

class SInterfaceBaseFactory
  {
  S_INTERFACE_FACTORY_TYPE(SInterfaceBase);
XProperties:
  XProperty(xsize, referenceCount, setReferenceCount);
  XROProperty(bool, deleteOnNoReferences);

public:
  SInterfaceBaseFactory(bool del) : _referenceCount(0), _deleteOnNoReferences(del) { }
  virtual SInterfaceBase *classInterface(SProperty *) { return 0; }
  };

#define S_INTERFACE_TYPE(typeId) public: \
  enum { InterfaceTypeId = SInterfaceTypes::typeId }; \
  virtual xuint32 interfaceTypeId() const { return InterfaceTypeId; } \
  private:

class SInterfaceBase
#ifdef S_PROPERTY_USER_DATA
    : public SProperty::UserData
#endif
  {
  S_INTERFACE_TYPE(Invalid)

#ifdef S_PROPERTY_USER_DATA
  bool onPropertyDelete(SProperty *) X_OVERRIDE { return true; }
#endif
  };


#define S_STATIC_INTERFACE_TYPE(type, interfaceTypeId) public: \
  S_INTERFACE_TYPE(interfaceTypeId) \
  S_INTERFACE_FACTORY_TYPE(type)

class SStaticInterfaceBase : public SInterfaceBase, public SInterfaceBaseFactory
  {
public:
  SStaticInterfaceBase(bool deleteOnNoReferences) : SInterfaceBaseFactory(deleteOnNoReferences) { }
  virtual SInterfaceBase *classInterface(SProperty *) { return this; }
  bool onPropertyDelete(SProperty *) { return false; }
  };

#endif // SINTERFACE_H
