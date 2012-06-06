#ifndef SDATABASE_H
#define SDATABASE_H

#include "sglobal.h"
#include "sentity.h"
#include "sbaseproperties.h"
#include "XBucketAllocator"
#include "sloader.h"
#include "shandler.h"

class SChange;

class SHIFT_EXPORT SDatabase : public SEntity, public SHandler
  {
  S_ENTITY(SDatabase, SEntity, 0);

public:
  UnsignedIntProperty majorVersion;
  UnsignedIntProperty minorVersion;
  UnsignedIntProperty revision;

  SDatabase();
  ~SDatabase();

  QVector<SProperty *> load(const QString &type, QIODevice *device, SPropertyContainer *loadRoot);
  void save(const QString &type, QIODevice *device, SEntity *saveRoot, bool readable, bool includeRoot);

  static QChar pathSeparator();

  XAllocatorBase *persistantAllocator()
    {
    return _memory;
    }

  bool stateStorageEnabled() const { return _stateStorageEnabled; }
  void setStateStorageEnabled(bool enable) { _stateStorageEnabled = enable; }

protected:
  void initiateInheritedDatabaseType(const SPropertyInformation *info);

private:
  SProperty *createDynamicProperty(const SPropertyInformation *info, SPropertyContainer *parentToBe, SPropertyInstanceInformationInitialiser *inst);
  void deleteDynamicProperty(SProperty *);
  void deleteProperty(SProperty *);

  InstanceInformation _instanceInfoData;

  void initiateProperty(SProperty *);
  void postInitiateProperty(SProperty *);
  void initiatePropertyFromMetaData(SPropertyContainer *prop, const SPropertyInformation *mD);
  void uninitiateProperty(SProperty *thisProp);
  void uninitiatePropertyFromMetaData(SPropertyContainer *container, const SPropertyInformation *mD);

  XAllocatorBase *_memory;

  bool _stateStorageEnabled;

  friend class SProperty;
  friend class SPropertyContainer;
  friend class SPropertyContainer::TreeChange;
  };

S_PROPERTY_INTERFACE(SDatabase)

#endif // SDATABASE_H
