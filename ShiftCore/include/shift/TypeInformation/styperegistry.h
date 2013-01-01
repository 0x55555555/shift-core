#ifndef STYPEREGISTRY_H
#define STYPEREGISTRY_H

#include "shift/sglobal.h"
#include "XSet"

namespace Shift
{

class PropertyNameArg;
class PropertyGroup;
class PropertyInformation;

class SHIFT_EXPORT TypeRegistry
  {
public:
  class Observer
    {
  public:
    virtual void typeAdded(const PropertyInformation *) = 0;
    virtual void typeRemoved(const PropertyInformation *) = 0;
    };

  static void addTypeObserver(Observer *o);
  static void removeTypeObserver(Observer *o);

  static void initiate();
  static void terminate();

  static Eks::AllocatorBase *allocator();

  static const Eks::Vector<const PropertyGroup *> &groups();
  static const Eks::Vector<const PropertyInformation *> &types();

  static void addPropertyGroup(PropertyGroup &);
  static void addType(const PropertyInformation *);

  static const PropertyInformation *findType(const PropertyNameArg &);

private:
  TypeRegistry();
  X_DISABLE_COPY(TypeRegistry);

  static void internalAddType(const PropertyInformation *);

  friend class PropertyGroup;
  };

}

#endif // STYPEREGISTRY_H
