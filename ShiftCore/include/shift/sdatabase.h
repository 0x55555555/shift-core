#ifndef SDATABASE_H
#define SDATABASE_H

#include "XBucketAllocator"
#include "shift/sglobal.h"
#include "shift/sentity.h"
#include "shift/Properties/sbaseproperties.h"
#include "shift/Serialisation/sloader.h"
#include "shift/Changes/shandler.h"

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

  bool stateStorageEnabled() const { return _stateStorageEnabled; }
  void setStateStorageEnabled(bool enable) { _stateStorageEnabled = enable; }

protected:
  void initiateInheritedDatabaseType(const PropertyInformation *info);

private:
  Property *createDynamicProperty(const PropertyInformation *info, PropertyContainer *parentToBe, PropertyInstanceInformationInitialiser *inst);
  void deleteDynamicProperty(Property *);
  void deleteProperty(Property *);

  DynamicInstanceInformation _instanceInfoData;

  void initiateProperty(Property *);
  void postInitiateProperty(Property *);
  void initiatePropertyFromMetaData(PropertyContainer *prop, const PropertyInformation *mD);
  void uninitiateProperty(Property *thisProp);
  void uninitiatePropertyFromMetaData(PropertyContainer *container, const PropertyInformation *mD);

  Eks::AllocatorBase *_memory;

  bool _stateStorageEnabled;

  friend class Property;
  friend class PropertyContainer;
  friend class PropertyContainer::TreeChange;
  };


class SHIFT_EXPORT StateStorageBlock
  {
public:
  StateStorageBlock(bool enable, Database *h) : _db(h), _oldValue(h->stateStorageEnabled())
    {
    _db->setStateStorageEnabled(enable);
    }

  ~StateStorageBlock()
    {
    _db->setStateStorageEnabled(_oldValue);
    }

private:
  Database *_db;
  bool _oldValue;
  };

}

S_PROPERTY_INTERFACE(Shift::Database)

#endif // SDATABASE_H
