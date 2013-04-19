#ifndef SBASEPROPERTIES_H
#define SBASEPROPERTIES_H

#include "shift/sentity.h"
#include "shift/Properties/sproperty.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/spropertyinstanceinformation.h"
#include "shift/Changes/schange.h"
#include "shift/Changes/sobserver.h"
#include "shift/Changes/shandler.h"
#include "shift/Changes/spropertychanges.h"

#if X_ASSERTS_ENABLED
# include "shift/sdatabase.h"
#endif

#include "XStringBuffer"
#include "XMathVector"
#include "XColour"
#include "XQuaternion"
#include "QtCore/QByteArray"
#include "QtCore/QUuid"

namespace Shift
{


typedef Eks::Vector<Eks::String> SStringVector;

#if X_QT_INTEROP

Q_DECLARE_METATYPE(SStringVector)

SHIFT_EXPORT QTextStream &operator<<(QTextStream &s, xuint8 v);
SHIFT_EXPORT QTextStream &operator>>(QTextStream &s, xuint8 &v);

SHIFT_EXPORT QTextStream &operator>>(QTextStream &s, SStringVector &);
SHIFT_EXPORT QTextStream &operator<<(QTextStream &s, const SStringVector &v);

SHIFT_EXPORT QTextStream &operator>>(QTextStream &s, QUuid &);
SHIFT_EXPORT QTextStream &operator<<(QTextStream &s, const QUuid &v);

#endif

namespace detail
{
template <typename T>void getDefault(T *)
  {
  }

void getDefault(xuint8 *t);
void getDefault(xint32 *t);
void getDefault(xint64 *t);
void getDefault(xuint32 *t);
void getDefault(xuint64 *t);
void getDefault(float *t);
void getDefault(double *t);
void getDefault(Eks::Vector2D *t);
void getDefault(Eks::Vector3D *t);
void getDefault(Eks::Vector4D *t);
void getDefault(Eks::Quaternion *t);
}

template <typename T>
class PODInterface
  {
  };

namespace detail
{
template <typename T> class BasePODPropertyTraits;
template <typename T> class PODPropertyTraits;
}

template <typename PROP, typename POD> class PODPropertyVariantInterface : public PropertyVariantInterface
  {
public:
#if X_QT_INTEROP
  virtual Eks::String asString(const Attribute *p) const
    {
    QString d;
      {
      QTextStream s(&d);
      s << p->uncheckedCastTo<PROP>()->value();
      }
    return Eks::String(d);
    }
  virtual QVariant asVariant(const Attribute *p) const
    {
    return QVariant::fromValue<POD>(p->uncheckedCastTo<PROP>()->value());
    }

  virtual void setVariant(Attribute *p, const QVariant &v) const
    {
    p->uncheckedCastTo<PROP>()->assign(v.value<POD>());
    }
#endif
  };

template <typename T, typename DERIVED> class PODPropertyBase : public Property
  {
protected:
  XPropertyMember(T, value);

protected:
  class ComputeChange : public Property::DataChange
    {
    S_CHANGE_TYPED(ComputeChange, Property::DataChange, Change::ComputeBase, T);

  public:
    ComputeChange(PODPropertyBase<T, DERIVED> *prop)
      : Property::DataChange(prop)
      {
      xAssert(!prop->database()->stateStorageEnabled());
      }

    bool apply()
      {
      property()->postCompute();
      return true;
      }

  private:
    bool unApply()
      {
      xAssertFail();
      return true;
      }
    bool inform(bool)
      {
      if(property()->entity())
        {
        property()->entity()->informDirtyObservers(property());
        }
      return true;
      }
    };

public:
  typedef T PODType;

  class ComputeLock
    {
  public:
    typedef typename PODPropertyBase<T, DERIVED>::ComputeChange Change;

    ComputeLock(PODPropertyBase<T, DERIVED> *ptr) : _ptr(ptr)
      {
      xAssert(ptr);
      _data = &_ptr->_value;
      }
    ~ComputeLock()
      {
      PropertyDoChangeNonLocal(Change, _ptr, _ptr);
      }

    T* data()
      {
      return _data;
      }

    T *operator->()
      {
      return _data;
      }

    ComputeLock &operator=(const T &x)
      {
      *_data = x;
      return *this;
      }

  private:
    PODPropertyBase<T, DERIVED> *_ptr;
    T* _data;
    };

  const T &operator()() const
    {
    preGet();
    return _value;
    }

  const T &value() const
    {
    preGet();
    return _value;
    }

protected:
  friend class ComputeLock;
  };

template <typename T> class PODProperty : public PODPropertyBase<T, PODProperty<T>>
  {
  typedef PODProperty<T> PODPropertyType;
  S_PROPERTY(PODPropertyType, Property, 0);

public:
  typedef Shift::detail::PODPropertyTraits<PODPropertyType> Traits;
  friend class Traits;

  class EmbeddedInstanceInformation : public Property::EmbeddedInstanceInformation
    {
  XProperties:
    XByRefProperty(T, defaultValue, setDefault);

  public:
    EmbeddedInstanceInformation()
      {
      detail::getDefault(&_defaultValue);
      }

    virtual void initiateAttribute(Attribute *propertyToInitiate) const
      {
      Property::EmbeddedInstanceInformation::initiateAttribute(propertyToInitiate);
      propertyToInitiate->uncheckedCastTo<PODPropertyType>()->_value = defaultValue();
      }

    virtual void setDefaultValueFromString(const Eks::String &val)
      {
      Eks::String::Buffer s(&val);
      Eks::String::IStream stream(&s);
      xAssertFail();
      //stream >> _defaultValue;
      }

    void setDefaultValue(const T &val)
      {
      _defaultValue = val;
      }
    };

  class Lock
    {
  public:
    Lock(PODPropertyType *ptr) : _ptr(ptr)
      {
      xAssert(ptr);
      _oldData = _ptr->value();
      _data = &_ptr->_value;
      }
    ~Lock()
      {
      PropertyDoChange(Change, _oldData, *_data, _ptr);
      _data = 0;
      }

    T* data()
      {
      return _data;
      }

  private:
    PODPropertyType *_ptr;
    T* _data;
    T _oldData;
    };

  void assign(const T &in);

private:
  class ComputeChange : public Property::DataChange
    {
    S_CHANGE_TYPED(ComputeChange, Property::DataChange, Change::ComputeChange, T);

  public:
    ComputeChange(PODPropertyType *prop)
      : Property::DataChange(prop)
      {
#if X_ASSERTS_ENABLED
      xAssert(!prop->database()->stateStorageEnabled());
#endif
      }

  private:
    bool apply()
      {
      return true;
      }
    bool unApply()
      {
      xAssertFail();
      return true;
      }
    bool inform(bool)
      {
      if(property()->entity())
        {
        property()->entity()->informDirtyObservers(property());
        }
      return true;
      }
    };

  class Change : public ComputeChange
    {
    S_CHANGE_TYPED(Change, ComputeChange, Change::DataChange, T);

  XProperties:
    XRORefProperty(T, before);
    XRORefProperty(T, after);

  public:
    Change(const T &b, const T &a, PODPropertyType *prop)
      : ComputeChange(prop), _before(b), _after(a)
      { }

    bool apply()
      {
      Attribute *prop = ComputeChange::property();
      PODPropertyType* d = prop->uncheckedCastTo<PODPropertyType>();
      d->_value = after();
      ComputeChange::property()->postSet();
      return true;
      }

    bool unApply()
      {
      Attribute *prop = ComputeChange::property();
      PODPropertyType* d = prop->uncheckedCastTo<PODPropertyType>();
      d->_value = before();
      ComputeChange::property()->postSet();
      return true;
      }

    bool inform(bool)
      {
      Entity *ent = ComputeChange::property()->entity();
      if(ent)
        {
        ent->informDirtyObservers(ComputeChange::property());
        }
      return true;
      }
    };

  friend class Lock;
  };

#define DEFINE_POD_PROPERTY(EXPORT_MODE, name, type, typeID) \
class EXPORT_MODE name : public Shift::PODProperty<type> { public: \
  name &operator=(const type &in) { \
    assign(in); \
    return *this; } \
  static void assignBetween(const Shift::Attribute *p, Shift::Attribute *l ); }; \
template <> class Shift::PODInterface <type> { public: typedef name Type; \
  static void assign(name* s, const type& val) { s->assign(val); } \
  static const type& value(const name* s) { return s->value(); } };

#define IMPLEMENT_POD_PROPERTY(type, grp) S_IMPLEMENT_PROPERTY_EXPLICIT(PODProperty<type>, type ## Property, grp) S_DEFAULT_TYPE_INFORMATION(PODProperty<type>)

DEFINE_POD_PROPERTY(SHIFT_EXPORT, BoolProperty, xuint8, 100);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, IntProperty, xint32, 101);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, LongIntProperty, xint64, 102);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, UnsignedIntProperty, xuint32, 103);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, LongUnsignedIntProperty, xuint64, 104);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, FloatProperty, float, 105);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, DoubleProperty, double, 106);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, Vector2DProperty, Eks::Vector2D, 107);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, Vector3DProperty, Eks::Vector3D, 108);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, Vector4DProperty, Eks::Vector4D, 109);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, QuaternionProperty, Eks::Quaternion, 110);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, StringPropertyBase, Eks::String, 111);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, ColourProperty, Eks::Colour, 112);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, ByteArrayProperty, QByteArray, 113);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, UuidPropertyBase, QUuid, 115);

DEFINE_POD_PROPERTY(SHIFT_EXPORT, StringArrayProperty, SStringVector, 114);

class SHIFT_EXPORT StringProperty : public StringPropertyBase
  {
public:
  class EmbeddedInstanceInformation : public StringPropertyBase::EmbeddedInstanceInformation
    {
  public:
    void setDefaultValue(const Eks::String &val)
      {
      setDefault(val);
      }
    };

  S_PROPERTY(StringProperty, StringPropertyBase, 0);
  StringProperty &operator=(const Eks::String &in)
    {
    assign(in);
    return *this;
    }
  };

class SHIFT_EXPORT UuidProperty : public UuidPropertyBase
  {
public:
  class EmbeddedInstanceInformation : public UuidPropertyBase::EmbeddedInstanceInformation
    {
  public:
    virtual void initiateAttribute(Property *propertyToInitiate) const
      {
      Property::EmbeddedInstanceInformation::initiateAttribute(propertyToInitiate);
      propertyToInitiate->uncheckedCastTo<UuidProperty>()->_value = QUuid::createUuid();
      }
    };

  S_PROPERTY(UuidProperty, UuidPropertyBase, 0);
  };

template <typename T> class FlagsProperty : public IntProperty
  {
public:
  void setFlag(T t, bool onOff)
    {
    Eks::Flags<T> val(_value);
    val.setFlag(t, onOff);
    assign(*val);
    }
  };

class SHIFT_EXPORT FilenameProperty : public StringProperty
  {
  S_PROPERTY(FilenameProperty, StringProperty, 0);

public:
  };

// specific pod interface for bool because it is actually a uint8.
template <> class PODInterface <bool> { public: typedef BoolProperty Type; \
  static void assign(BoolProperty* s, const bool &val) { s->assign(val); } \
  static const xuint8 &value(const BoolProperty* s) { return s->value(); } };

}

#if X_QT_INTEROP

Q_DECLARE_METATYPE(QUuid)

#endif

#define EnumProperty IntProperty


namespace Shift
{

template <typename T>
    void PODProperty<T>::assign(const T &in)
  {
  PropertyDoChange(Change, PODPropertyBase<T>::_value, in, this);
  }
}


#endif // SBASEPROPERTIES_H
