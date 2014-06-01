#ifndef SINTERFACE_H
#define SINTERFACE_H

#include "shift/TypeInformation/styperegistry.h"

#define S_INTERFACE_FACTORY_TYPE(interfaceType) public: \
  typedef interfaceType InterfaceType; \
  private:

#define S_INTERFACE_TYPE(typeId) public: \
  enum { InterfaceTypeId = SInterfaceTypes::typeId }; \
  private:

#define S_STATIC_INTERFACE_TYPE(type, typeID) public: \
  S_INTERFACE_TYPE(typeID) \
  xuint32 interfaceTypeId() const X_OVERRIDE { return InterfaceTypeId; } \
  S_INTERFACE_FACTORY_TYPE(type)

namespace Shift
{

class InterfaceBase;

class InterfaceBaseFactory
  {
  S_INTERFACE_FACTORY_TYPE(InterfaceBase);

public:
  virtual ~InterfaceBaseFactory() { }

  virtual xuint32 interfaceTypeId() const = 0;
  virtual InterfaceBase *classInterface(Attribute *) { return 0; }
  };

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

class StaticInterfaceBase : public InterfaceBase, public InterfaceBaseFactory
  {
public:
  InterfaceBase *classInterface(Attribute *) X_OVERRIDE { return this; }
  };

}

#endif // SINTERFACE_H
