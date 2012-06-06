#ifndef SBASEPROPERTIES_H
#define SBASEPROPERTIES_H

#include "sproperty.h"
#include "spropertyinformation.h"

#include "XVector2D"
#include "XVector3D"
#include "XVector4D"
#include "XColour"
#include "XQuaternion"
#include "sentity.h"
#include "schange.h"
#include "sobserver.h"
#include "sinterface.h"
#include "sinterfaces.h"
#include "QByteArray"

typedef QVector<QString> SStringVector;
Q_DECLARE_METATYPE(SStringVector)

SHIFT_EXPORT QTextStream &operator<<(QTextStream &s, xuint8 v);
SHIFT_EXPORT QTextStream &operator>>(QTextStream &s, xuint8 &v);

SHIFT_EXPORT QTextStream &operator>>(QTextStream &s, SStringVector &);
SHIFT_EXPORT QTextStream &operator<<(QTextStream &s, const SStringVector &v);


template <typename T>
class SPODInterface
  {
  };

template <typename PROP, typename POD> class PODPropertyVariantInterface : public SPropertyVariantInterface
  {
public:
  PODPropertyVariantInterface() : SPropertyVariantInterface(true) { }
  virtual QString asString(const SProperty *p) const
    {
    QString d;
      {
      QTextStream s(&d);
      s << p->uncheckedCastTo<PROP>()->value();
      }
    return d;
    }
  virtual QVariant asVariant(const SProperty *p) const
    {
    return QVariant::fromValue<POD>(p->uncheckedCastTo<PROP>()->value());
    }

  virtual void setVariant(SProperty *p, const QVariant &v) const
    {
    p->uncheckedCastTo<PROP>()->assign(v.value<POD>());
    }
  };

template <typename T, typename DERIVED> class SPODPropertyBase : public SProperty
  {
protected:
  XPropertyMember(T, value);

public:
  class ComputeLock
    {
  public:
    ComputeLock(SPODPropertyBase<T, DERIVED> *ptr) : _ptr(ptr)
      {
      xAssert(ptr);
      _data = &_ptr->_value;
      }
    ~ComputeLock()
      {
      _ptr->handler()->doChange<ComputeChange>(_ptr);
      }

    T* data()
      {
      return _data;
      }

  private:
    SPODPropertyBase<T, DERIVED> *_ptr;
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

  static bool shouldSavePropertyValue(const SProperty *)
    {
    return false;
    }

protected:
  class ComputeChange : public SProperty::DataChange
    {
    S_CHANGE(ComputeChange, SProperty::DataChange, 155);

  public:
    ComputeChange(SPODPropertyBase<T, DERIVED> *prop)
      : SProperty::DataChange(prop)
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

  friend class ComputeLock;
  };

template <typename T, typename DERIVED> class SPODProperty : public SPODPropertyBase<T, DERIVED>
  {
public:
  class InstanceInformation : public SProperty::InstanceInformation
    {
  XProperties:
    XByRefProperty(T, defaultValue, setDefault);

  public:
    InstanceInformation(const T& d) : _defaultValue(d)
      {
      }

    virtual void initiateProperty(SProperty *propertyToInitiate) const
      {
      SProperty::InstanceInformation::initiateProperty(propertyToInitiate);
      propertyToInitiate->uncheckedCastTo<DERIVED>()->_value = defaultValue();
      }

    virtual void setDefaultValue(const QString &val)
      {
      QString cpyVal(val);
      QTextStream s(&cpyVal);
      s >> _defaultValue;
      }
    };

  class Lock
    {
  public:
    Lock(SPODProperty<T, DERIVED> *ptr) : _ptr(ptr)
      {
      xAssert(ptr);
      _oldData = _ptr->value();
      _data = &_ptr->_value;
      }
    ~Lock()
      {
      _ptr->handler()->doChange<Change>(_oldData, *_data, _ptr);
      _data = 0;
      }

    T* data()
      {
      return _data;
      }

  private:
    SPODProperty<T, DERIVED> *_ptr;
    T* _data;
    T _oldData;
    };

  static void saveProperty(const SProperty *p, SSaver &l )
    {
    SProperty::saveProperty(p, l);
    const DERIVED *ptr = p->uncheckedCastTo<DERIVED>();
    writeValue(l, ptr->_value);
    }

  static SProperty *loadProperty(SPropertyContainer *parent, SLoader &l)
    {
    SProperty *prop = SProperty::loadProperty(parent, l);
    DERIVED *ptr = prop->uncheckedCastTo<DERIVED>();
    readValue(l, ptr->_value);
    return prop;
    }

  static bool shouldSavePropertyValue(const SProperty *p)
    {
    const DERIVED *ptr = p->uncheckedCastTo<DERIVED>();

    if(SProperty::shouldSavePropertyValue(p))
      {
      if(ptr->value() != ptr->instanceInformation()->defaultValue())
        {
        return true;
        }
      }

    return false;
    }

  void assign(const T &in);

private:
  class ComputeChange : public SProperty::DataChange
    {
    S_CHANGE(ComputeChange, SProperty::DataChange, DERIVED::TypeId);

  public:
    ComputeChange(SPODProperty<T, DERIVED> *prop)
      : SProperty::DataChange(prop)
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
    Change(const T &b, const T &a, SPODProperty<T, DERIVED> *prop)
      : ComputeChange(prop), _before(b), _after(a)
      { }

  private:
    bool apply()
      {
      property()->uncheckedCastTo<DERIVED>()->_value = after();
      property()->postSet();
      return true;
      }

    bool unApply()
      {
      property()->uncheckedCastTo<DERIVED>()->_value = before();
      property()->postSet();
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

  friend class Lock;
  };

#define DEFINE_POD_PROPERTY(EXPORT_MODE, name, type, defaultDefault, typeID) \
class EXPORT_MODE name : public SPODProperty<type, name> { \
public: class InstanceInformation : public SPODProperty<type, name>::InstanceInformation \
    { public: \
    InstanceInformation() : SPODProperty<type, name>::InstanceInformation(defaultDefault) { } }; \
  enum { TypeId = typeID }; \
  typedef type PODType; \
  S_PROPERTY(name, SProperty, 0); \
  name(); \
  name &operator=(const type &in) { \
    assign(in); \
    return *this; } \
  static void assignProperty(const SProperty *p, SProperty *l ); }; \
template <> class SPODInterface <type> { public: typedef name Type; \
  static void assign(name* s, const type& val) { s->assign(val); } \
  static const type& value(const name* s) { return s->value(); } }; \
S_PROPERTY_INTERFACE(name)

#define IMPLEMENT_POD_PROPERTY(name, grp) \
  S_IMPLEMENT_PROPERTY(name, grp) \
  void name::createTypeInformation(SPropertyInformationTyped<name> *info, \
      const SPropertyInformationCreateData &data) { \
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
DEFINE_POD_PROPERTY(SHIFT_EXPORT, Vector2DProperty, XVector2D, XVector2D(0.0f, 0.0f), 107);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, Vector3DProperty, XVector3D, XVector3D(0.0f, 0.0f, 0.0f), 108);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, Vector4DProperty, XVector4D, XVector4D(0.0f, 0.0f, 0.0f, 0.0f), 109);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, QuaternionProperty, XQuaternion, XQuaternion(), 110);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, StringPropertyBase, QString, "", 111);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, ColourProperty, XColour, XColour(0.0f, 0.0f, 0.0f, 1.0f), 112);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, ByteArrayProperty, QByteArray, QByteArray(), 113);

DEFINE_POD_PROPERTY(SHIFT_EXPORT, StringArrayProperty, SStringVector, SStringVector(), 114);

class SHIFT_EXPORT StringProperty : public StringPropertyBase
  {
public:
  class InstanceInformation : public StringPropertyBase::InstanceInformation
    {
  public:
    InstanceInformation()
      {
      }
    void setDefaultValue(const QString &val)
      {
      setDefault(val);
      }
    };

  S_PROPERTY(StringProperty, StringPropertyBase, 0);
  StringProperty()
    {
    }
  StringProperty &operator=(const QString &in)
    {
    assign(in);
    return *this;
    }
  };

S_PROPERTY_INTERFACE(StringProperty)

class SHIFT_EXPORT FilenameProperty : public StringProperty
  {
  S_PROPERTY(FilenameProperty, StringProperty, 0);

public:
  FilenameProperty()
    {
    }
  };

S_PROPERTY_INTERFACE(FilenameProperty)

#define EnumProperty IntProperty

// specific pod interface for bool because it is actually a uint8.
template <> class SPODInterface <bool> { public: typedef BoolProperty Type; \
  static void assign(BoolProperty* s, const bool &val) { s->assign(val); } \
  static const xuint8 &value(const BoolProperty* s) { return s->value(); } };

template <> class SPODInterface <QStringList>
  {
public:
  typedef StringArrayProperty Type;

  static void assign(StringArrayProperty* s, const QStringList &val) { s->assign(val.toVector()); }
  static QStringList value(const StringArrayProperty* s) { return QStringList::fromVector(s->value()); }
  };



#include "sdatabase.h"

template <typename T, typename DERIVED> void SPODProperty<T, DERIVED>::assign(const T &in)
  {
  handler()->doChange<Change>(_value, in, this);
  }

#endif // SBASEPROPERTIES_H
