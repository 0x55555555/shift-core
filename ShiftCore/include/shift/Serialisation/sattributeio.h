#ifndef SATTRIBUTEIO_H
#define SATTRIBUTEIO_H

#include "shift/sglobal.h"
#include "shift/Utilities/spropertyname.h"
#include "Utilities/XMacroHelpers.h"
#include "Utilities/XProperty.h"
#include "Containers/XStringSimple.h"
#include "Containers/XStringBuffer.h"

namespace Shift
{
class Attribute;
class PropertyInformation;

/// \brief A symbol is a known key used to identifiy data in a serialised file.
///        Attribute interfaces should implement their own sub class, which holds
///        data appropriate for their serialisation style.
class SerialisationSymbol
  {
public:
  SerialisationSymbol() { }

private:
  X_DISABLE_COPY(SerialisationSymbol);
  };

/// \brief A serialisation value is a value sent from the emitter to the Attribute
///        interface, and can be turned into binary or Utf8 formats.
class SerialisationValue
  {
public:
  virtual bool hasUtf8() const = 0;
  virtual bool hasBinary() const = 0;
  virtual Eks::String asUtf8(Eks::AllocatorBase* a) const { xAssertFail(); return Eks::String(a); }
  virtual Eks::Vector<xuint8> asBinary(Eks::AllocatorBase* a) const { xAssertFail(); return Eks::Vector<xuint8>(a); }
  };

/// \brief Helper class for wrapping a typed value for Serialisation.
template <typename T> class TypedSerialisationValue : public SerialisationValue
  {
public:
  TypedSerialisationValue(const T *t) : _val(t) { }
  bool hasUtf8() const X_OVERRIDE { return true; }
  bool hasBinary() const X_OVERRIDE { return false; }
  Eks::String asUtf8(Eks::AllocatorBase*) const X_OVERRIDE;

private:
  const T *_val;
  };

/// \brief Attribute saver and loader base class, allows access to symbols used for writing and reading.
class AttributeIO
  {
public:
  typedef SerialisationSymbol Symbol;

  virtual const Symbol &modeSymbol() const = 0;
  virtual const Symbol &inputSymbol() const = 0;
  virtual const Symbol &valueSymbol() const = 0;
  };

/// \brief Util for Saving an attribute
class AttributeSaver : public AttributeIO
  {
public:
  /// \brief write a value for the attribute, with symbol [id].
  virtual void writeValue(const Symbol &id, const SerialisationValue& value) = 0;

  template <typename T> void write(const Symbol &id, const T& t)
    {
    TypedSerialisationValue<T> val(&t);

    writeValue(id, val);
    }
  };

namespace detail
{
template <typename T> struct ValueExtractor
  {
  static bool extract(const Eks::String &ret, T &val)
    {
    Eks::String::Buffer buf(&ret);
    Eks::String::IStream str(&buf);

    str >> val;
    return true;
    }
  };

template <xsize S, typename A> struct ValueExtractor<Eks::StringBase<Eks::String::Char, S, A>>
  {
  static bool extract(const Eks::String &ret, Eks::StringBase<Eks::String::Char, S, A> &val)
    {
    val = ret;
    return true;
    }
  };

template <> struct ValueExtractor<Eks::String>
  {
  static bool extract(const Eks::String &ret, Eks::String &val)
    {
    val = ret;
    return true;
    }
  };
}

/// \brief Util for loading an attribute
class AttributeLoader : public AttributeIO
  {
public:
  /// \brief read a value for the attribute, with symbol [id].
  virtual const SerialisationValue& readValue(const Symbol &id) = 0;
  virtual Eks::AllocatorBase *temporaryAllocator() = 0;

  virtual Attribute *existingAttribute() = 0;

  virtual const Name& name() = 0;
  virtual const PropertyInformation *type() = 0;

  virtual void resolveInputAfterLoad(Property *prop, const Eks::String &path) = 0;

  template <typename T> bool read(const Symbol &id, T &t)
    {
    const SerialisationValue &val = readValue(id);

    if (val.hasUtf8())
      {
      return detail::ValueExtractor<T>::extract(val.asUtf8(temporaryAllocator()), t);
      }

    return false;
    }
  };

template <typename T>
Eks::String TypedSerialisationValue<T>::asUtf8(Eks::AllocatorBase* a) const
  {
  Eks::String ret(a);
  Eks::String::Buffer buf(&ret);
  Eks::String::OStream str(&buf);

  str << *_val;
  return ret;
  }

template <> class SHIFT_EXPORT TypedSerialisationValue<QUuid> :  public SerialisationValue
  {
public:
  TypedSerialisationValue(const QUuid *t);

  bool hasUtf8() const X_OVERRIDE { return true; }
  bool hasBinary() const X_OVERRIDE { return false; }

  Eks::String asUtf8(Eks::AllocatorBase *a) const;

private:
  const QUuid *_val;
  };


}

#endif // SATTRIBUTEIO_H
