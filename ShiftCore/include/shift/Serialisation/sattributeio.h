#ifndef SATTRIBUTEIO_H
#define SATTRIBUTEIO_H

#include "shift/sglobal.h"
#include "Utilities/XMacroHelpers.h"
#include "Utilities/XProperty.h"
#include "Containers/XStringSimple.h"
#include "Containers/XStringBuffer.h"

namespace Shift
{
class Attribute;

class SerialisationSymbol
  {
public:
  SerialisationSymbol() { }

private:
  X_DISABLE_COPY(SerialisationSymbol);
  };

class SerialisationValue
  {
public:
  virtual Eks::String asUtf8(Eks::AllocatorBase*) const = 0;
  virtual Eks::Vector<xuint8> asBinary(Eks::AllocatorBase*) const = 0;
  };

template <typename T> class TypedSerialisationValue : public SerialisationValue
  {
public:
  TypedSerialisationValue(const T &t) : _val(t) { }
  Eks::String asUtf8(Eks::AllocatorBase*) const X_OVERRIDE;
  Eks::Vector<xuint8> asBinary(Eks::AllocatorBase*) const X_OVERRIDE;

private:
  const T &_val;
  };

class AttributeIO
  {
public:
  typedef SerialisationSymbol Symbol;

  virtual const Symbol &modeSymbol() = 0;
  virtual const Symbol &inputSymbol() = 0;
  virtual const Symbol &valueSymbol() = 0;
  };

class AttributeSaver : public AttributeIO
  {
public:
  /// \brief write a value for the attribute, with symbol [id].
  virtual void writeValue(const Symbol &id, const SerialisationValue& value) = 0;

  template <typename T> void write(const Symbol &id, const T& t)
    {
    TypedSerialisationValue<T> val(t);

    writeValue(id, val);
    }
  };

template <typename T>
Eks::String TypedSerialisationValue<T>::asUtf8(Eks::AllocatorBase* a) const
  {
  Eks::String ret(a);
  Eks::String::Buffer buf(&ret);
  Eks::String::OStream str(&buf);

  str << _val;
  return ret;
  }

template <typename T>
Eks::Vector<xuint8> TypedSerialisationValue<T>::asBinary(Eks::AllocatorBase*) const
  {
  xAssertFail();
  return Eks::Vector<xuint8>();
  }


template <> class SHIFT_EXPORT TypedSerialisationValue<QUuid> :  public SerialisationValue
  {
public:
  TypedSerialisationValue(const QUuid &t);
  Eks::String asUtf8(Eks::AllocatorBase *a) const;
  Eks::Vector<xuint8> asBinary(Eks::AllocatorBase*) const;

private:
  const QUuid &_val;
  };


}

#endif // SATTRIBUTEIO_H
