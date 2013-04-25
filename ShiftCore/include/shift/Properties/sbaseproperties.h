#ifndef SBASEPROPERTIES_H
#define SBASEPROPERTIES_H

#include "shift/sentity.h"
#include "shift/Properties/sproperty.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/spropertyinstanceinformation.h"
#include "shift/Changes/schange.h"
#include "shift/Changes/sobserver.h"
#include "shift/Changes/shandler.h"

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
template <typename T> class PODChange;
template <typename T> class PODComputeChange;
template <typename T> class PODLock;
template <typename T> class PODComputeLock;
template <typename T, int IsAttribute, int IsFull> class PODPropertyTraits;
template <typename T> class PODEmbeddedInstanceInformation;

void podPreGet(const Property *);
void podPreGet(const Attribute *);
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
    detail::podPreGet(this);
    return _value;
    }

  const T &value() const
    {
    detail::podPreGet(this);
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

#define EnumProperty IntProperty

template <typename T, DataMode Mode>
    void Data<T, Mode>::assign(const T &in)
  {
  PropertyDoChange(Change, Data<T, Mode>::_value, in, this);
  }

}

#if X_QT_INTEROP

Q_DECLARE_METATYPE(QUuid)

#endif



#endif // SBASEPROPERTIES_H
