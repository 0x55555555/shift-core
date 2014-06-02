#ifndef SCHANGE_H
#define SCHANGE_H

#include "shift/sglobal.h"
#include "Utilities/XProperty.h"
#include "Containers/XStringSimple.h"
#include "shift/Utilities/smetatype.h"

/// \def S_CHANGE_ID(CHANGE_TYPE) Find the change type id for the class [CHANGE_TYPE].
/// \internal
#define S_CHANGE_ID(CHANGE_TYPE) (Change::getChangeTypeId<typename CHANGE_TYPE::SubType>(CHANGE_TYPE::Type))

/// \def S_CASTABLE(myName, superName, rootName) Declare a castable change,
///      passing [myName] the name of the Change, [superName] the parent Change.
/// \internal
#define S_CASTABLE(myName, superName) \
    public: \
      virtual const Shift::Change *castToType( xuint32 id ) const { if( id == S_CHANGE_ID(myName) ){return this;}else{return superName::castToType( id );} } \
    private:

/// \def S_CHANGE(cl, supCl, baseType) Add this to the top of a class definition to make it a castable change.
///      [cl] is the name of the castable class.
///      [supCl] is the name of the parent class.
///      [baseType] is the changes id.
#define S_CHANGE(cl, supCl, baseType) \
  public: enum {Type = baseType}; \
  S_CASTABLE(cl, supCl)

/// \def S_CHANGE_TYPED(cl, supCl, baseType, type) Add this to the top of a class definition to make it a typed castable change. Use this for templated change classes.
///      [cl] is the name of the castable class.
///      [supCl] is the name of the parent class.
///      [baseType] is the changes id.
///      [type] is the type of this change.
#define S_CHANGE_TYPED(cl, supCl, baseType, type) \
  public: enum {Type = baseType}; \
  typedef type SubType; \
  S_CASTABLE(cl, supCl)

namespace Shift
{

/// \brief Changes are stored by the database and represent an atomic operation on the db.
///
///        Examples of a change are: changing the name of an object, creating and destroying a connection or setting an attribute
class SHIFT_EXPORT Change
  {
public:
  /// \brief The base change types
  enum BaseChangeTypes
    {
    BaseChange, ///< This
    NameChange, ///< Changing a name
    ConnectionChange, ///< Creating or destroying a connection
    TreeChange, ///< Inserting or removing a dynamic attribute
    BaseDataChange, ///< The base type for changing data
    ComputeChange, ///< A change created during a computation
    DataChange, ///< A data change to an attribute

    /// \brief The first index for adding custom types
    BaseChangeCount
    };

  /// \brief This Change's type.
  enum
    {
    Type = Change::BaseChange
    };

  /// \brief This Change's sub type.
  typedef void SubType;

  /// \brief Given a type[T] and a change type id [t] find a unique number that represents them.
  template <typename T> static xuint32 getChangeTypeId(xuint32 t)
    {
    int id = detail::MetaType::id<T>();
    xAssert(id < std::numeric_limits<xuint16>::max());

    return (t<<16) + id;
    }

  virtual ~Change();

  /// \brief Called to apply this change to the data it references
  virtual bool apply() = 0;
  /// \brief Called to unapply (undo) this change from the data it references
  ///
  ///        The change should assume the database is in the state it left it after its apply() call.
  virtual bool unApply() = 0;
  /// \brief Called to inform watchers that the data has been changed.
  virtual bool inform(bool backwards) = 0;

  /// \brief Cast a change, equivalent to a static_cast<>
  template <typename T> inline T *uncheckedCastTo()
    {
    xAssert(castTo<T>());
    return static_cast<T*>(this);
    }

  /// \brief Cast a change, equivalent to a static_cast<>
  template <typename T> inline const T *uncheckedCastTo() const
    {
    xAssert(castTo<T>());
    return static_cast<const T*>(this);
    }

  /// \brief Cast a change, equivalent to a dynamic_cast<>
  template <typename T>inline T *castTo()
    {
    return static_cast<T*>(castToType(S_CHANGE_ID(T)));
    }

  /// \brief Cast a change, equivalent to a dynamic_cast<>
  template <typename T>inline const T *castTo() const
    {
    return static_cast<const T *>(castToType(S_CHANGE_ID(T)));
    }

  /// \brief Try to cast this change to the type [id]
  Change *castToType(xuint32 id);
  /// \brief Try to cast this change to the type [id]
  virtual const Change *castToType(xuint32 id) const;
  };


}

#endif // SCHANGE_H
