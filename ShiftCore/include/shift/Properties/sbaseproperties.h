#ifndef SBASEPROPERTIES_H
#define SBASEPROPERTIES_H

#include "shift/sentity.h"
#include "shift/Properties/sproperty.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/spropertyinstanceinformation.h"
#include "shift/TypeInformation/sinterface.h"
#include "shift/TypeInformation/sinterfaces.h"
#include "shift/Changes/schange.h"
#include "shift/Changes/sobserver.h"
#include "shift/Changes/shandler.h"
#include "shift/Changes/spropertychanges.h"

#include "XStringBuffer"
#include "XMathVector"
#include "XColour"
#include "XQuaternion"
#include "QByteArray"

namespace Shift
{


typedef Eks::Vector<Eks::String> SStringVector;

#if X_QT_INTEROP

Q_DECLARE_METATYPE(SStringVector)

SHIFT_EXPORT QTextStream &operator<<(QTextStream &s, xuint8 v);
SHIFT_EXPORT QTextStream &operator>>(QTextStream &s, xuint8 &v);

SHIFT_EXPORT QTextStream &operator>>(QTextStream &s, SStringVector &);
SHIFT_EXPORT QTextStream &operator<<(QTextStream &s, const SStringVector &v);

#endif

template <typename T>
class PODInterface
  {
  };

template <typename PROP, typename POD> class PODPropertyVariantInterface : public PropertyVariantInterface
  {
public:
  PODPropertyVariantInterface() : PropertyVariantInterface(true) { }
#if X_QT_INTEROP
  virtual Eks::String asString(const Property *p) const
    {
    xAssertFail(); // this is bad code.
    QString d;
      {
      QTextStream s(&d);
      s << p->uncheckedCastTo<PROP>()->value();
      }
    return Eks::String(d);
    }
  virtual QVariant asVariant(const Property *p) const
    {
    return QVariant::fromValue<POD>(p->uncheckedCastTo<PROP>()->value());
    }

  virtual void setVariant(Property *p, const QVariant &v) const
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
    S_CHANGE(ComputeChange, Property::DataChange, 155);

  public:
    ComputeChange(PODPropertyBase<T, DERIVED> *prop)
      : Property::DataChange(prop)
      {
      xAssert(!prop->database()->stateStorageEnabled());
      }

  private:
    bool apply()
      {
      property()->postSet();
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

public:
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

  static bool shouldSavePropertyValue(const Property *)
    {
    return false;
    }

protected:
  friend class ComputeLock;
  };

template <typename T, typename DERIVED> class PODProperty : public PODPropertyBase<T, DERIVED>
  {
public:
  class EmbeddedInstanceInformation : public Property::EmbeddedInstanceInformation
    {
  XProperties:
    XByRefProperty(T, defaultValue, setDefault);

  public:
    EmbeddedInstanceInformation(const T& d) : _defaultValue(d)
      {
      }

    virtual void initiateProperty(Property *propertyToInitiate) const
      {
      Property::EmbeddedInstanceInformation::initiateProperty(propertyToInitiate);
      propertyToInitiate->uncheckedCastTo<DERIVED>()->_value = defaultValue();
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
    Lock(PODProperty<T, DERIVED> *ptr) : _ptr(ptr)
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
    PODProperty<T, DERIVED> *_ptr;
    T* _data;
    T _oldData;
    };

  static void saveProperty(const Property *p, Saver &l )
    {
    Property::saveProperty(p, l);
    const DERIVED *ptr = p->uncheckedCastTo<DERIVED>();
    writeValue(l, ptr->_value);
    }

  static Property *loadProperty(PropertyContainer *parent, Loader &l)
    {
    Property *prop = Property::loadProperty(parent, l);
    DERIVED *ptr = prop->uncheckedCastTo<DERIVED>();
    readValue(l, ptr->_value);
    return prop;
    }

  static bool shouldSavePropertyValue(const Property *p)
    {
    const DERIVED *ptr = p->uncheckedCastTo<DERIVED>();

    if(Property::shouldSavePropertyValue(p))
      {
      using ::operator!=;

      if(ptr->isDynamic() ||
         ptr->value() != ptr->embeddedInstanceInformation()->defaultValue())
        {
        return true;
        }
      }

    return false;
    }

  void assign(const T &in);

private:
  class ComputeChange : public Property::DataChange
    {
    S_CHANGE(ComputeChange, Property::DataChange, DERIVED::TypeId);

  public:
    ComputeChange(PODProperty<T, DERIVED> *prop)
      : Property::DataChange(prop)
      {
      xAssert(!prop->database()->stateStorageEnabled());
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
    S_CHANGE(Change, ComputeChange, DERIVED::TypeId + 1000);

  XProperties:
    XRORefProperty(T, before);
    XRORefProperty(T, after);

  public:
    Change(const T &b, const T &a, PODProperty<T, DERIVED> *prop)
      : ComputeChange(prop), _before(b), _after(a)
      { }

    bool apply()
      {
      Property *prop = ComputeChange::property();
      DERIVED* d = prop->uncheckedCastTo<DERIVED>();
      d->_value = after();
      ComputeChange::property()->postSet();
      return true;
      }

    bool unApply()
      {
      Property *prop = ComputeChange::property();
      DERIVED* d = prop->uncheckedCastTo<DERIVED>();
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

#define DEFINE_POD_PROPERTY(EXPORT_MODE, name, type, defaultDefault, typeID) \
class EXPORT_MODE name : public PODProperty<type, name> { \
public: class EmbeddedInstanceInformation : \
  public PODProperty<type, name>::EmbeddedInstanceInformation \
    { public: \
    EmbeddedInstanceInformation() \
    : PODProperty<type, name>::EmbeddedInstanceInformation(defaultDefault) { } }; \
  enum { TypeId = typeID }; \
  typedef type PODType; \
  S_PROPERTY(name, Property, 0); \
  name(); \
  name &operator=(const type &in) { \
    assign(in); \
    return *this; } \
  static void assignProperty(const Property *p, Property *l ); }; \
template <> class PODInterface <type> { public: typedef name Type; \
  static void assign(name* s, const type& val) { s->assign(val); } \
  static const type& value(const name* s) { return s->value(); } };

#define IMPLEMENT_POD_PROPERTY(name, grp) \
  S_IMPLEMENT_PROPERTY(name, grp) \
  void name::createTypeInformation(PropertyInformationTyped<name> *info, \
      const PropertyInformationCreateData &data) { \
    if(data.registerInterfaces) { \
    info->addStaticInterface(new PODPropertyVariantInterface<name, name::PODType>()); } } \
  name::name() { }

DEFINE_POD_PROPERTY(SHIFT_EXPORT, BoolProperty, xuint8, 0, 100);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, IntProperty, xint32, 0, 101);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, LongIntProperty, xint64, 0, 102);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, UnsignedIntProperty, xuint32, 0, 103);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, LongUnsignedIntProperty, xuint64, 0, 104);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, FloatProperty, float, 0.0f, 105);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, DoubleProperty, double, 0.0, 106);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, Vector2DProperty, Eks::Vector2D, Eks::Vector2D(0.0f, 0.0f), 107);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, Vector3DProperty, Eks::Vector3D, Eks::Vector3D(0.0f, 0.0f, 0.0f), 108);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, Vector4DProperty, Eks::Vector4D, Eks::Vector4D(0.0f, 0.0f, 0.0f, 0.0f), 109);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, QuaternionProperty, Eks::Quaternion, Eks::Quaternion::Identity(), 110);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, StringPropertyBase, Eks::String, "", 111);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, ColourProperty, Eks::Colour, Eks::Colour(0.0f, 0.0f, 0.0f, 1.0f), 112);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, ByteArrayProperty, QByteArray, QByteArray(), 113);

DEFINE_POD_PROPERTY(SHIFT_EXPORT, StringArrayProperty, SStringVector, SStringVector(), 114);

class SHIFT_EXPORT StringProperty : public StringPropertyBase
  {
public:
  class EmbeddedInstanceInformation : public StringPropertyBase::EmbeddedInstanceInformation
    {
  public:
    EmbeddedInstanceInformation()
      {
      }

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

template <typename T> class FlagsProperty : public IntProperty
  {
public:
  void setFlag(T t, bool onOff)
    {
    XFlags<T> val(_value);
    val.setFlag(t, onOff);
    assign(*val);
    }
  };

class SHIFT_EXPORT FilenameProperty : public StringProperty
  {
  S_PROPERTY(FilenameProperty, StringProperty, 0);

public:
  FilenameProperty()
    {
    }
  };


#define EnumProperty IntProperty

// specific pod interface for bool because it is actually a uint8.
template <> class PODInterface <bool> { public: typedef BoolProperty Type; \
  static void assign(BoolProperty* s, const bool &val) { s->assign(val); } \
  static const xuint8 &value(const BoolProperty* s) { return s->value(); } };
}

#include "shift/sdatabase.h"

namespace Shift
{

template <typename T, typename DERIVED> void PODProperty<T, DERIVED>::assign(const T &in)
  {
  PropertyDoChange(Change, PODPropertyBase<T, DERIVED>::_value, in, this);
  }
}

S_PROPERTY_INTERFACE(Shift::BoolProperty)
S_PROPERTY_INTERFACE(Shift::IntProperty)
S_PROPERTY_INTERFACE(Shift::LongIntProperty)
S_PROPERTY_INTERFACE(Shift::UnsignedIntProperty)
S_PROPERTY_INTERFACE(Shift::LongUnsignedIntProperty)
S_PROPERTY_INTERFACE(Shift::FloatProperty)
S_PROPERTY_INTERFACE(Shift::DoubleProperty)
S_PROPERTY_INTERFACE(Shift::Vector2DProperty)
S_PROPERTY_INTERFACE(Shift::Vector3DProperty)
S_PROPERTY_INTERFACE(Shift::Vector4DProperty)
S_PROPERTY_INTERFACE(Shift::QuaternionProperty)
S_PROPERTY_INTERFACE(Shift::StringPropertyBase)
S_PROPERTY_INTERFACE(Shift::ColourProperty)
S_PROPERTY_INTERFACE(Shift::ByteArrayProperty)
S_PROPERTY_INTERFACE(Shift::StringArrayProperty)
S_PROPERTY_INTERFACE(Shift::StringProperty)
S_PROPERTY_INTERFACE(Shift::FilenameProperty)


#endif // SBASEPROPERTIES_H
