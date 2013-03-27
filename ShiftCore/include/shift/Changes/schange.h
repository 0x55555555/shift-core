#ifndef SCHANGE_H
#define SCHANGE_H

#include "shift/sglobal.h"
#include "XProperty"

// Casting macros
#define S_CASTABLE( myName, superName, myID, rootName ) \
    public: \
    virtual const rootName *castToType( xuint32 id ) const { if( id == myID ){return this;}else{return superName::castToType( id );} } \
    private:

// Casting macro for XObject to use (neater to have it here)
#define S_CASTABLE_ROOT( myName, myID, getTID ) \
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
      return static_cast<T*>( castToType( getTID ) ); \
      } \
    template <typename T>inline const T *castTo() const \
      { \
      return static_cast<const T *>( castToType( getTID ) ); \
      } \
    private:

#define S_CHANGE_ROOT(id) public: enum {Type = (id)}; virtual xuint32 type() const { return id; } S_CASTABLE_ROOT( Change, id, static_cast<T*>(0)->Type )

#define S_CHANGE(cl, supCl, id) public: enum {Type = (id)}; virtual xuint32 type() const { return id; } S_CASTABLE( cl, supCl, id, Shift::Change )

namespace Shift
{

class Change
  {
  S_CHANGE_ROOT(1);
public:
  virtual ~Change() { }
  virtual bool apply() = 0;
  virtual bool unApply() = 0;
  virtual bool inform(bool backwards) = 0;
  };

}

#endif // SCHANGE_H
