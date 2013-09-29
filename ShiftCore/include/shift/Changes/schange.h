#ifndef SCHANGE_H
#define SCHANGE_H

#include "shift/sglobal.h"
#include "XProperty"
#include "XStringSimple"

#define S_CHANGE_ID(CHANGE_TYPE) (Change::getChangeTypeId<typename CHANGE_TYPE::SubType>(CHANGE_TYPE::Type))

// Casting macros
#define S_CASTABLE( myName, superName, rootName ) \
    public: \
      virtual const rootName *castToType( xuint32 id ) const { if( id == S_CHANGE_ID(myName) ){return this;}else{return superName::castToType( id );} } \
    private:

// Casting macro for XObject to use (neater to have it here)
#define S_CASTABLE_ROOT( myName, myID ) \
    public: \
    myName *castToType( xuint32 id ) { return (myName*)((const myName*)this)->castToType(id); } \
    virtual const myName *castToType( xuint32 id ) const { if( id == myID ){return this;}else{return 0;} } \
    template <typename T>inline T *uncheckedCastTo() \
      { \
      xAssert( castTo<T>() ); \
      return static_cast<T*>( this ); \
      } \
    template <typename T>inline const T *uncheckedCastTo() const \
      { \
      xAssert( castTo<T>() ); \
      return static_cast<const T*>( this ); \
      } \
    template <typename T>inline T *castTo() \
      { \
      return static_cast<T*>( castToType( S_CHANGE_ID(T) ) ); \
      } \
    template <typename T>inline const T *castTo() const \
      { \
      return static_cast<const T *>( castToType( S_CHANGE_ID(T) ) ); \
      } \
    private:


#define S_CHANGE_ROOT \
  public: enum { Type = Change::BaseChange }; \
  typedef void SubType; \
  S_CASTABLE_ROOT( Change, Type )

#define S_CHANGE(cl, supCl, baseType) \
  public: enum {Type = baseType}; \
  S_CASTABLE( cl, supCl, Shift::Change )

#define S_CHANGE_TYPED(cl, supCl, baseType, type) \
  public: enum {Type = baseType}; \
  typedef type SubType; \
  S_CASTABLE( cl, supCl, Shift::Change )

namespace Shift
{
namespace detail
{
class SHIFT_EXPORT MetaType
  {
public:
  template <typename T>static xuint32 id()
    {
    static xuint32 id = newId();
    return id;
    }

  template <typename T> static void appendTypeName(Eks::String &str)
    {
    str.appendType(id<T>());
    }

private:
  static xuint32 newId();
  };
}

class Change
  {
public:
  enum BaseChangeTypes
  {
    BaseChange,
    NameChange,
    ConnectionChange,
    TreeChange,
    BaseDataChange,
    ComputeChange,
    DataChange
  };

  S_CHANGE_ROOT

public:
  template <typename T> static xuint32 getChangeTypeId(xuint32 t)
    {
    int id = detail::MetaType::id<T>();
    xAssert(id < X_UINT16_SENTINEL);

    return (t<<16) + id;
    }

  virtual ~Change() { }
  virtual bool apply() = 0;
  virtual bool unApply() = 0;
  virtual bool inform(bool backwards) = 0;
  };


}

#endif // SCHANGE_H
