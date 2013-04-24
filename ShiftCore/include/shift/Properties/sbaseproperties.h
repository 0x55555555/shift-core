#ifndef SBASEPROPERTIES_H
#define SBASEPROPERTIES_H

#include "shift/sentity.h"
#include "shift/Properties/sproperty.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/spropertyinstanceinformation.h"
#include "shift/Changes/schange.h"
#include "shift/Changes/sobserver.h"
#include "shift/Changes/shandler.h"

#include "XTemporaryAllocator"
#include "shift/Changes/spropertychanges.h"
#include "shift/TypeInformation/spropertytraits.h"
#include "shift/TypeInformation/spropertygroup.h"

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

template <typename T>
class PODInterface
  {
  };

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

namespace detail
{
template <typename T> void getDefault(T *)
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

void assignTo(const Shift::Attribute *, Shift::Attribute *)
  {
  xAssertFail();
  }

template <typename T, int IsAttribute, int IsFull> class PODPropertyTraits
  : public PropertyBaseTraits
  {
public:
  static bool shouldSaveValue(const Attribute *)
    {
    return false;
    }

  static void preGet(const Attribute *)
    {
    }

  static void preGet(const Property *prop)
    {
    prop->preGet();
    }

  enum
    {
    assign = 0
    };
  };

template <typename T, int IsAttribute> class PODPropertyTraits<T, IsAttribute, true>
  : public PropertyBaseTraits
  {
public:
  static void save(const Attribute *p, Saver &l)
    {
    PropertyBaseTraits::save(p, l);
    const T *ptr = p->uncheckedCastTo<T>();
    writeValue(l, ptr->_value);
    }

  static Attribute *load(Container *parent, Loader &l)
    {
    Attribute *prop = PropertyBaseTraits::load(parent, l);
    T *ptr = prop->uncheckedCastTo<T>();
    readValue(l, ptr->_value);
    return prop;
    }

  static bool shouldSaveValue(const Attribute *p)
    {
    const T *ptr = p->uncheckedCastTo<T>();

    if(PropertyBaseTraits::shouldSaveValue(p))
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

  static void preGet(const Attribute *attr)
    {
    }

  static void preGet(const Property *prop)
    {
    prop->preGet();
    }

  static void assign(const Shift::Attribute *p, Shift::Attribute *l)
    {
    T *t = l->uncheckedCastTo<T>();
    Shift::detail::assignTo(p, t);
    }
  };

template <typename T> class PODComputeChange : public Property::DataChange
  {
  S_CHANGE_TYPED(PODComputeChange, Property::DataChange, Change::ComputeChange, T);

public:
  PODComputeChange(T *prop)
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

template <typename T> class PODComputeLock
  {
public:
  typedef typename T::ComputeChange Change;

  PODComputeLock(T *ptr) : _ptr(ptr)
    {
    xAssert(ptr);
    _data = &_ptr->_value;
    }

  ~PODComputeLock()
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

  PODComputeLock &operator=(const T &x)
    {
    *_data = x;
    return *this;
    }

private:
  T *_ptr;
  typename T::PODType *_data;
  };

template <typename T> class PODEmbeddedInstanceInformation
    : public Property::EmbeddedInstanceInformation
  {
XProperties:
  typedef typename T::PODType PODType;
  XByRefProperty(PODType, defaultValue, setDefault);

public:
  PODEmbeddedInstanceInformation()
    {
    detail::getDefault(&_defaultValue);
    }

  virtual void initiateAttribute(Attribute *propertyToInitiate) const
    {
    Property::EmbeddedInstanceInformation::initiateAttribute(propertyToInitiate);
    propertyToInitiate->uncheckedCastTo<T>()->_value = defaultValue();
    }

  virtual void setDefaultValueFromString(const Eks::String &val)
    {
    Eks::String::Buffer s(&val);
    Eks::String::IStream stream(&s);
    xAssertFail();
    }

  void setDefaultValue(const T &val)
    {
    _defaultValue = val;
    }
  };

template <typename T> class PODLock
  {
public:
  PODLock(T *ptr) : _ptr(ptr)
    {
    xAssert(ptr);
    _oldData = _ptr->value();
    _data = &_ptr->_value;
    }
  ~PODLock()
    {
    PropertyDoChange(Change, _oldData, *_data, _ptr);
    _data = 0;
    }

  T* data()
    {
    return _data;
    }

private:
  T *_ptr;
  typename T::PODType* _data;
  typename T::PODType _oldData;
  };


template <typename T> class PODChange : public PODComputeChange<T>
  {
  S_CHANGE_TYPED(PODChange, PODComputeChange, Change::DataChange, T);

XProperties:
  typedef typename T::PODType PODType;
  XRORefProperty(PODType, before);
  XRORefProperty(PODType, after);

public:
  PODChange(const PODType &b, const PODType &a, T *prop)
    : PODComputeChange(prop), _before(b), _after(a)
    { }

  bool apply()
    {
    Attribute *prop = PODComputeChange::property();
    T* d = prop->uncheckedCastTo<T>();
    d->_value = after();
    PODComputeChange::property()->postSet();
    return true;
    }

  bool unApply()
    {
    Attribute *prop = PODComputeChange::property();
    T* d = prop->uncheckedCastTo<T>();
    d->_value = before();
    PODComputeChange::property()->postSet();
    return true;
    }

  bool inform(bool)
    {
    Entity *ent = PODComputeChange::property()->entity();
    if(ent)
      {
      ent->informDirtyObservers(PODComputeChange::property());
      }
    return true;
    }
  };

}

enum DataMode
  {
  AttributeData,
  ComputedData,
  FullData,

  DataModeCount
  };

template <typename T, DataMode Mode=FullData> class Data
    : public Eks::IfElse<Mode != AttributeData, Property, Attribute>::Type
  {
public:
  typedef typename Eks::IfElse<Mode != AttributeData, Property, Attribute>::Type ParentType;
  typedef Data<T, Mode> PODPropertyType;

  enum
    {
    IsAttribute = Mode > AttributeData,
    IsCopyable = Mode >= FullData
    };

  typedef detail::PODPropertyTraits<PODPropertyType, IsAttribute, IsCopyable> Traits;
  typedef detail::PODEmbeddedInstanceInformation<PODPropertyType> EmbeddedInstanceInformation;
  typedef typename ParentType::DynamicInstanceInformation DynamicInstanceInformation;

  S_PROPERTY(PODPropertyType, ParentType, 0);

public:
  typedef T PODType;

  typedef detail::PODChange<PODPropertyType> Change;
  typedef detail::PODComputeChange<PODPropertyType> ComputeChange;


  void assign(const T &in);

  const T &operator()() const
    {
    Traits::preGet(this);
    return _value;
    }

  const T &value() const
    {
    Traits::preGet(this);
    return _value;
    }

protected:
  XPropertyMember(T, value);

  typedef detail::PODLock<PODPropertyType> Lock;
  typedef detail::PODComputeLock<T> ComputeLock;

  friend class ComputeLock;
  friend class ComputeChange;
  friend class Change;
  friend class Traits;
  friend class Lock;
  friend class EmbeddedInstanceInformation;

  static PropertyGroup::Information _staticTypeInformation;
  };

#define DEFINE_POD_PROPERTY(EXPORT_MODE, name, type, typeID) \
typedef Shift::Data<type> name; \
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
DEFINE_POD_PROPERTY(SHIFT_EXPORT, ColourProperty, Eks::Colour, 112);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, ByteArrayProperty, QByteArray, 113);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, UuidPropertyBase, QUuid, 115);
DEFINE_POD_PROPERTY(SHIFT_EXPORT, StringProperty, Eks::String, 112);

DEFINE_POD_PROPERTY(SHIFT_EXPORT, StringArrayProperty, SStringVector, 114);


namespace detail
{
template<> class PODEmbeddedInstanceInformation<Data<QUuid>>
        : public Property::EmbeddedInstanceInformation
  {
  typedef QUuid PODType;

public:

  virtual void initiateAttribute(Attribute *propertyToInitiate) const
    {
    Property::EmbeddedInstanceInformation::initiateAttribute(propertyToInitiate);
    propertyToInitiate->uncheckedCastTo<Data<QUuid>>()->_value = QUuid::createUuid();
    }

  QUuid defaultValue() const
    {
    return QUuid();
    }
  };


template <typename T> class Flags : public IntProperty
  {
public:
  void setFlag(T t, bool onOff)
    {
    Eks::Flags<T> val(_value);
    val.setFlag(t, onOff);
    assign(*val);
    }
  };

// specific pod interface for bool because it is actually a uint8.
/*template <> class PODInterface <bool> { public: typedef BoolProperty Type; \
  static void assign(BoolProperty* s, const bool &val) { s->assign(val); } \
  static const xuint8 &value(const BoolProperty* s) { return s->value(); } };
*/
}

#define EnumProperty IntProperty

template <typename T, DataMode Mode>
    void Data<T, Mode>::assign(const T &in)
  {
  PropertyDoChange(Change, Data<T, Mode>::_value, in, this);
  }

template <typename T, DataMode Mode>
    const PropertyInformation *Data<T, Mode>::staticTypeInformation()
  {
  return _staticTypeInformation.information;
  }

template <typename T, DataMode Mode>
    void Data<T, Mode>::createTypeInformation(PropertyInformationTyped<Data<T, Mode>> *,
                                      const PropertyInformationCreateData &)
  {
  }


template <typename T, DataMode Mode>
    const Shift::PropertyInformation *Data<T, Mode>::
      bootstrapStaticTypeInformation(Eks::AllocatorBase *allocator)
  {
  typedef Data<T, Mode> ThisType;
  Shift::detail::checkType<ThisType>();

  Eks::TemporaryAllocator temp(TypeRegistry::temporaryAllocator());
  Eks::String name(&temp);

  const char *modeType = "Full";
  if(Mode == ComputedData)
    {
    modeType = "Computed";
    }
  else if(Mode == AttributeData)
    {
    modeType = "Attribute";
    }

  name.appendType(modeType);
  name.appendType(QMetaType::typeName(qMetaTypeId<T>()));

  Shift::PropertyInformationTyped<ThisType>::bootstrapTypeInformation(
        &_staticTypeInformation.information,
        name.data(),
        ThisType::ParentType::bootstrapStaticTypeInformation(allocator), allocator);

  return staticTypeInformation();
  }

namespace detail
{
void assignTo(const Attribute *f, Data<Eks::String> *to);
void assignTo(const Attribute *f, ColourProperty *to);
void assignTo(const Attribute *f, Vector4DProperty *to);
void assignTo(const Attribute *f, Vector3DProperty *to);
void assignTo(const Attribute *f, Vector2DProperty *to);
void assignTo(const Attribute *f, DoubleProperty *to);
void assignTo(const Attribute *f, FloatProperty *to);
void assignTo(const Attribute *f, LongUnsignedIntProperty *to);
void assignTo(const Attribute *f, UnsignedIntProperty *to);
void assignTo(const Attribute *f, LongIntProperty *to);
void assignTo(const Attribute *f, IntProperty *to);
void assignTo(const Attribute *f, BoolProperty *to);
}
}

#if X_QT_INTEROP

Q_DECLARE_METATYPE(QUuid)

#endif



#endif // SBASEPROPERTIES_H
