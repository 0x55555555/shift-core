#ifndef SINTERFACE_H
#define SINTERFACE_H

#include "shift/Properties/sproperty.h"

#define S_INTERFACE_FACTORY_TYPE(interfaceType) public: \
  typedef interfaceType InterfaceType; \
  private:

namespace Shift
{

class InterfaceBase;

class InterfaceBaseFactory
  {
  S_INTERFACE_FACTORY_TYPE(InterfaceBase);
XProperties:
  XProperty(xsize, referenceCount, setReferenceCount);
  XROProperty(bool, deleteOnNoReferences);

public:
  InterfaceBaseFactory(bool del) : _referenceCount(0), _deleteOnNoReferences(del) { }
  virtual InterfaceBase *classInterface(Property *) { return 0; }
  };

#define S_INTERFACE_TYPE(typeId) public: \
  enum { InterfaceTypeId = SInterfaceTypes::typeId }; \
  virtual xuint32 interfaceTypeId() const { return InterfaceTypeId; } \
  private:

class InterfaceBase
#ifdef S_PROPERTY_USER_DATA
    : public Property::UserData
#endif
  {
  S_INTERFACE_TYPE(Invalid)

#ifdef S_PROPERTY_USER_DATA
  bool onPropertyDelete(Property *) X_OVERRIDE { return true; }
#endif
  };


#define S_STATIC_INTERFACE_TYPE(type, interfaceTypeId) public: \
  S_INTERFACE_TYPE(interfaceTypeId) \
  S_INTERFACE_FACTORY_TYPE(type)

class StaticInterfaceBase : public InterfaceBase, public InterfaceBaseFactory
  {
public:
  StaticInterfaceBase(bool deleteOnNoReferences) : InterfaceBaseFactory(deleteOnNoReferences) { }
  virtual InterfaceBase *classInterface(Property *) { return this; }
  bool onPropertyDelete(Property *) { return false; }
  };

}

#endif // SINTERFACE_H
