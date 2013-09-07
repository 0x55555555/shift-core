#ifndef SGLOBAL_H
#define SGLOBAL_H

#include "XGlobal"

/** \mainpage Shift

    \version 0.5

    \section intro_sec Introduction

    Shift is a database/graph library for saving and loading and computing data.

    \section gettingstarted_sec Getting Started

    To start using shift, you need to create a Database, and some data!

    \sa Database
    \sa Entity
 */

#if defined(SHIFTCORE_BUILD)
#  define SHIFT_EXPORT Q_DECL_EXPORT
#else
#  define SHIFT_EXPORT Q_DECL_IMPORT
#endif

#ifdef X_PROFILING_ENABLED
# include "XProfiler"
# define ShiftCoreProfileScope 1043
# define ShiftDataModelProfileScope 1044
# define SProfileFunction X_EVENT_FUNCTION;
# define SProfileScopedBlock(mess) X_EVENT_FUNCTION;
#else
# define SProfileFunction
# define SProfileScopedBlock(mess)
#endif

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
class Attribute;
class Property;
class Observer;
class PropertyGroup;

enum DataMode
  {
  AttributeData,
  ComputedData,
  FullData,

  DataModeCount
  };

PropertyGroup &propertyGroup();
}

#endif // SHIFT_GLOBAL_H
