#ifndef SPROPERTYNAME_H
#define SPROPERTYNAME_H

#include "shift/sglobal.h"
#include "Containers/XStringSimple.h"

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

#endif // SPROPERTYNAME_H
