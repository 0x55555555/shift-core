#ifndef SGLOBAL_H
#define SGLOBAL_H

#include "XGlobal"
#include "XProfiler"

#if defined(SHIFTCORE_BUILD)
#  define SHIFT_EXPORT Q_DECL_EXPORT
#else
#  define SHIFT_EXPORT Q_DECL_IMPORT
#endif

#define ShiftCoreProfileScope 1043
#define ShiftDataModelProfileScope 1044
#define SProfileFunction XProfileFunctionBase(ShiftCoreProfileScope)
#define SProfileScopedBlock(mess) XProfileScopedBlockBase(ShiftCoreProfileScope, mess)

#define S_DEFINE_USER_DATA_TYPE(name, typeId) namespace SUserDataTypes { static const xuint32 name = typeId; }
S_DEFINE_USER_DATA_TYPE(Invalid, 0)
S_DEFINE_USER_DATA_TYPE(InterfaceUserDataType, 0)

#define S_DEFINE_INTERFACE_TYPE(name, typeId) namespace SInterfaceTypes { static const xuint32 name = typeId; }
S_DEFINE_INTERFACE_TYPE(Invalid, 0)
S_DEFINE_INTERFACE_TYPE(PropertyVariantInterface, 1)
S_DEFINE_INTERFACE_TYPE(HandlerInterface, 2)
S_DEFINE_INTERFACE_TYPE(PropertyPositionInterface, 3)
S_DEFINE_INTERFACE_TYPE(PropertyColourInterface, 4)
S_DEFINE_INTERFACE_TYPE(PropertyConnectionInterface, 5)

namespace Shift
{

class Entity;
class Property;
class Observer;
class TreeObserver;
class DirtyObserver;
class ConnectionObserver;
class PropertyGroup;

typedef Eks::Vector<Observer*> Observers;
typedef Eks::Vector<TreeObserver*> TreeObservers;
typedef Eks::Vector<DirtyObserver*> DirtyObservers;
typedef Eks::Vector<ConnectionObserver*> ConnectionObservers;

PropertyGroup &propertyGroup();
}

#endif // SHIFT_GLOBAL_H
