#ifndef SGLOBAL_H
#define SGLOBAL_H

#include "XGlobal"
#include "XList"
#include "XProfiler"

#if defined(SHIFT_BUILD)
#  define SHIFT_EXPORT Q_DECL_EXPORT
#else
#  define SHIFT_EXPORT Q_DECL_IMPORT
#endif

#define ShiftCoreProfileScope 1043
#define ShiftDataModelProfileScope 1044
#define SProfileFunction XProfileFunctionBase(ShiftCoreProfileScope)
#define SProfileScopedBlock(mess) XProfileScopedBlockBase(ShiftCoreProfileScope, mess)

class SEntity;
class SProperty;
class SObserver;
class STreeObserver;
class SDirtyObserver;
class SConnectionObserver;

typedef XVector<SObserver*> SObservers;
typedef XVector<STreeObserver*> STreeObservers;
typedef XVector<SDirtyObserver*> SDirtyObservers;
typedef XVector<SConnectionObserver*> SConnectionObservers;

typedef XVector<SEntity*> SEntities;
typedef XVector<const SEntity*> SConstEntities;

#define S_DEFINE_USER_DATA_TYPE(name, typeId) namespace SUserDataTypes { static const xuint32 name = typeId; }
S_DEFINE_USER_DATA_TYPE(Invalid, 0)
S_DEFINE_USER_DATA_TYPE(InterfaceUserDataType, 0)

#define S_DEFINE_INTERFACE_TYPE(name, typeId) namespace SInterfaceTypes { static const xuint32 name = typeId; }
S_DEFINE_INTERFACE_TYPE(Invalid, 0)
S_DEFINE_INTERFACE_TYPE(PropertyVariantInterface, 1)
S_DEFINE_INTERFACE_TYPE(SHandlerInterface, 2)
S_DEFINE_INTERFACE_TYPE(PropertyPositionInterface, 3)
S_DEFINE_INTERFACE_TYPE(PropertyColourInterface, 4)
S_DEFINE_INTERFACE_TYPE(PropertyConnectionInterface, 5)

class SPropertyGroup;
namespace Shift
{
SPropertyGroup &propertyGroup();
}

#endif // SHIFT_GLOBAL_H
