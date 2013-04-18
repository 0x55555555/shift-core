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

/** \brief A database.

  \section Introduction
  An Database represents the full database, it holds every element created. There should only need to be single
  database for most applications. A database can contain multiple Handlers, which can (optionally) manage their
  own changes (implementing Undo), or share some common Change architecture. A database should be treated at as
  an Entity, the only difference being unlike normal Entities it should be constructed by the user.

  \sa Handler

  \section Creating a Database
  Databases can be created on the stack. A database should be created after the TypeRegistry is initialised. As a
  database inherits from Entity, once created, entities can be added, and data can be built up.

  \section Deriving from Database
  Deriving from database is similar to deriving from Entity, except as the database is the root object, its type must
  be explicitly stated by calling initiateInheritedDatabaseType in the constructor with the typeInformation() of
  the derived type passed in.
 */
class SHIFT_EXPORT Database : public Entity, public Handler
  {
  S_ENTITY(Database, Entity, 0);

public:
  Database();
  ~Database();

  /// Simple interface to load data into the database
  /// \sa Loader
  Eks::Vector<Attribute *> load(const QString &type, QIODevice *device, Container *loadRoot);
  /// Simple interface to save data into the database
  /// \sa Saver
  void save(const QString &type, QIODevice *device, Entity *saveRoot, bool readable, bool includeRoot);

  /// The path separator used when forming string paths to properties.
  static const Eks::Char *pathSeparator();
  /// The path separator used when forming string paths to properties. Escaped for convenience.
  static const Eks::Char *escapedPathSeparator();

  /// The persistent allocator to use for creating properties and property data.
  Eks::AllocatorBase *persistantAllocator()
    {
    return _memory;
    }

  /// A temporary allocator root to be used when allocating temporary memory inside Shift, for example, during computation.
  Eks::TemporaryAllocatorCore *temporaryAllocator()
    {
    return _temporaryMemory;
    }

protected:
  /// Call this method from derived Database classes to ensure the hierarchy is set up correctly.
  void initiateInheritedDatabaseType(const PropertyInformation *info);

private:
  Attribute *createDynamicAttribute(const PropertyInformation *info, Container *parentToBe, PropertyInstanceInformationInitialiser *inst);
  void deleteDynamicAttribute(Attribute *);

  DynamicInstanceInformation _instanceInfoData;

  void initiateAttribute(Attribute *);
  void postInitiateAttribute(Attribute *);
  void initiateAttributeFromMetaData(Container *prop, const PropertyInformation *mD);
  void uninitiateAttribute(Attribute *thisProp);
  void uninitiateAttributeFromMetaData(Container *container, const PropertyInformation *mD);

  Eks::AllocatorBase *_memory;
  Eks::TemporaryAllocatorCore *_temporaryMemory;

  friend class Property;
  friend class Container;
  friend class Container::TreeChange;
  };

}

S_PROPERTY_INTERFACE(Shift::Database)

#endif // SDATABASE_H
