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

typedef Eks::StringBase<Eks::Char, NamePreallocSize> Name;

class SHIFT_EXPORT NameArg
  {
public:
  NameArg();
  NameArg(NameArg &&);
  NameArg& operator=(NameArg&& other);

#if X_QT_INTEROP
  NameArg(const QString &);
#endif

  template <typename C, xsize S, typename A>
  NameArg(const Eks::StringBase<C, S, A> &in)
    {
    xCompileTimeAssert(sizeof(C) == sizeof(Eks::Char));

    _type = TypeEks;
    _data.eks = in.data();
    _length = in.length();
    }

  NameArg(const Eks::Char *in)
    {
    _type = TypeEks;
    _data.eks = in;
    _length = Eks::StringBase<Eks::Char>::Traits::length(in);
    }

  void toName(Name &out) const;

  bool isEmpty() const;

  bool operator==(const NameArg &oth) const;
  bool operator==(const Name &oth) const;

private:
  X_DISABLE_COPY(NameArg)

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

#include "XScriptFunction.h"
#include "XConvertFromScript.h"

namespace XScript
{

namespace internal
{
template <> struct ConvertArg<const Shift::NameArg &, Shift::Name>
  {
  static Shift::Name conv(const Shift::NameArg &a)
    {
    Shift::Name n;
    a.toName(n);
    return n;
    }
  };
}

namespace Convert
{

namespace internal
{

template <> struct JSToNative<const Shift::NameArg &>
  {
  typedef Shift::Name ResultType;

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

template <> class TypeMatcher<const Shift::NameArg&, Shift::Name>
  {
public:
  static inline Shift::NameArg match(const Shift::Name *in, bool& valid)
    {
    valid = true;
    return *in;
    }
  };

}
}

Q_DECLARE_METATYPE(Shift::Name)

#endif // SPROPERTYNAME_H
