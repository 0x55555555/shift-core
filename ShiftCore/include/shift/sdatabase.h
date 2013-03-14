#ifndef SDATABASE_H
#define SDATABASE_H

#include "XBucketAllocator"
#include "shift/sglobal.h"
#include "shift/sentity.h"
#include "shift/Serialisation/sloader.h"
#include "shift/Changes/shandler.h"
#include "shift/TypeInformation/spropertyinstanceinformation.h"

namespace Eks
{
class TemporaryAllocatorCore;
}

namespace Shift
{

class Change;

class SHIFT_EXPORT Database : public Entity, public Handler
  {
  S_ENTITY(Database, Entity, 0);

public:
  Database();
  ~Database();

  QVector<Property *> load(const QString &type, QIODevice *device, PropertyContainer *loadRoot);
  void save(const QString &type, QIODevice *device, Entity *saveRoot, bool readable, bool includeRoot);

  static const Eks::Char *pathSeparator();
  static const Eks::Char *escapedPathSeparator();

  Eks::AllocatorBase *persistantAllocator()
    {
    return _memory;
    }

  Eks::TemporaryAllocatorCore *temporaryAllocator()
    {
    return _temporaryMemory;
    }

protected:
  void initiateInheritedDatabaseType(const PropertyInformation *info);

private:
  Property *createDynamicProperty(const PropertyInformation *info, PropertyContainer *parentToBe, PropertyInstanceInformationInitialiser *inst);
  void deleteDynamicProperty(Property *);

  DynamicInstanceInformation _instanceInfoData;

  void initiateProperty(Property *);
  void postInitiateProperty(Property *);
  void initiatePropertyFromMetaData(PropertyContainer *prop, const PropertyInformation *mD);
  void uninitiateProperty(Property *thisProp);
  void uninitiatePropertyFromMetaData(PropertyContainer *container, const PropertyInformation *mD);

  Eks::AllocatorBase *_memory;
  Eks::TemporaryAllocatorCore *_temporaryMemory;


  friend class Property;
  friend class PropertyContainer;
  friend class PropertyContainer::TreeChange;
  };

}

S_PROPERTY_INTERFACE(Shift::Database)

#endif // SDATABASE_H
