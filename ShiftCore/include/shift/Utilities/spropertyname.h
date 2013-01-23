#ifndef SPROPERTYNAME_H
#define SPROPERTYNAME_H

#include "shift/sglobal.h"
#include "XStringSimple"

namespace Shift
{

enum
  {
  NamePreallocSize = 32
  };

typedef Eks::StringBase<Eks::Char, NamePreallocSize> PropertyName;

class SHIFT_EXPORT PropertyNameArg
  {
public:
  PropertyNameArg();
  PropertyNameArg(PropertyNameArg &&);
  PropertyNameArg& operator=(PropertyNameArg&& other);

#if X_QT_INTEROP
  PropertyNameArg(const QString &);
#endif

  template <typename C, xsize S, typename A>
  PropertyNameArg(const Eks::StringBase<C, S, A> &in)
    {
    xCompileTimeAssert(sizeof(C) == sizeof(Eks::Char));

    _type = TypeEks;
    _data.eks = in.data();
    _length = in.length();
    }

  PropertyNameArg(const Eks::Char *in)
    {
    _type = TypeEks;
    _data.eks = in;
    _length = Eks::StringBase<Eks::Char>::Traits::length(in);
    }

  void toName(PropertyName &out) const;

  bool isEmpty() const;

  bool operator==(const PropertyNameArg &oth) const;
  bool operator==(const PropertyName &oth) const;

private:
  X_DISABLE_COPY(PropertyNameArg)

  enum
    {
    TypeEks,
#if X_QT_INTEROP
    TypeQt
#endif
    } _type;

  xsize _length;
  union
    {
    const Eks::Char *eks;
#if X_QT_INTEROP
    const QString *qt;
#endif
    } _data;
  };

}

#include "XConvertFromScript.h"

namespace XScript
{

namespace Convert
{

namespace internal
{

template <> struct JSToNative<const Shift::PropertyNameArg &>
  {
  typedef Shift::PropertyName ResultType;

  ResultType operator()(Value const &h) const
    {
    return h.toString();
    }
  };

}


namespace detail
{
template <typename T> class MatchOut
  {
  typedef typename std::remove_const<T>::type NoConst;
  typedef typename std::remove_reference<NoConst>::type Type;
  };
}

template <> class TypeMatcher<const Shift::PropertyNameArg&, Shift::PropertyName>
  {
public:
  static inline Shift::PropertyNameArg match(const Shift::PropertyName *in, bool& valid)
    {
    valid = true;
    return *in;
    }
  };

}
}

#endif // SPROPERTYNAME_H
