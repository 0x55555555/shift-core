#ifndef SBASEPROPERTIES_H
#define SBASEPROPERTIES_H

#include "shift/sglobal.h"
#include "shift/Properties/sdata.h"

#include "XStringBuffer"
#include "XMathVector"
#include "XColour"
#include "XQuaternion"
#include "QtCore/QByteArray"
#include "QtCore/QUuid"

namespace Shift
{

#define DEFINE_POD_PROPERTY(name, type) typedef Shift::Data<type> name;

typedef Eks::Vector<Eks::String> StringVector;
Q_DECLARE_METATYPE(xuint8);

DEFINE_POD_PROPERTY(BoolProperty, xuint8);
DEFINE_POD_PROPERTY(IntProperty, xint32);
DEFINE_POD_PROPERTY(LongIntProperty, xint64);
DEFINE_POD_PROPERTY(UnsignedIntProperty, xuint32);
DEFINE_POD_PROPERTY(LongUnsignedIntProperty, xuint64);
DEFINE_POD_PROPERTY(FloatProperty, float);
DEFINE_POD_PROPERTY(DoubleProperty, double);
DEFINE_POD_PROPERTY(Vector2DProperty, Eks::Vector2D);
DEFINE_POD_PROPERTY(Vector3DProperty, Eks::Vector3D);
DEFINE_POD_PROPERTY(Vector4DProperty, Eks::Vector4D);
DEFINE_POD_PROPERTY(QuaternionProperty, Eks::Quaternion);
DEFINE_POD_PROPERTY(ColourProperty, Eks::Colour);
DEFINE_POD_PROPERTY(ByteArrayProperty, QByteArray);
DEFINE_POD_PROPERTY(StringProperty, Eks::String);
DEFINE_POD_PROPERTY(StringArrayProperty, StringVector);

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

#define EnumProperty IntProperty

namespace detail
{
SHIFT_EXPORT void getDefault(xuint8 *t);
SHIFT_EXPORT void getDefault(xint32 *t);
SHIFT_EXPORT void getDefault(xint64 *t);
SHIFT_EXPORT void getDefault(xuint32 *t);
SHIFT_EXPORT void getDefault(xuint64 *t);
SHIFT_EXPORT void getDefault(float *t);
SHIFT_EXPORT void getDefault(double *t);
SHIFT_EXPORT void getDefault(Eks::Vector2D *t);
SHIFT_EXPORT void getDefault(Eks::Vector3D *t);
SHIFT_EXPORT void getDefault(Eks::Vector4D *t);
SHIFT_EXPORT void getDefault(Eks::Quaternion *t);

SHIFT_EXPORT void assignTo(const Attribute *, Attribute *);
SHIFT_EXPORT void assignTo(const Attribute *f, Data<Eks::String> *to);
SHIFT_EXPORT void assignTo(const Attribute *f, ColourProperty *to);
SHIFT_EXPORT void assignTo(const Attribute *f, Vector4DProperty *to);
SHIFT_EXPORT void assignTo(const Attribute *f, Vector3DProperty *to);
SHIFT_EXPORT void assignTo(const Attribute *f, Vector2DProperty *to);
SHIFT_EXPORT void assignTo(const Attribute *f, DoubleProperty *to);
SHIFT_EXPORT void assignTo(const Attribute *f, FloatProperty *to);
SHIFT_EXPORT void assignTo(const Attribute *f, LongUnsignedIntProperty *to);
SHIFT_EXPORT void assignTo(const Attribute *f, UnsignedIntProperty *to);
SHIFT_EXPORT void assignTo(const Attribute *f, LongIntProperty *to);
SHIFT_EXPORT void assignTo(const Attribute *f, IntProperty *to);
SHIFT_EXPORT void assignTo(const Attribute *f, BoolProperty *to);
}

}

#if X_QT_INTEROP

SHIFT_EXPORT QTextStream &operator<<(QTextStream &s, xuint8 v);
SHIFT_EXPORT QTextStream &operator>>(QTextStream &s, xuint8 &v);

SHIFT_EXPORT QTextStream &operator>>(QTextStream &s, Shift::StringVector &);
SHIFT_EXPORT QTextStream &operator<<(QTextStream &s, const Shift::StringVector &v);

SHIFT_EXPORT QTextStream &operator>>(QTextStream &s, QUuid &);
SHIFT_EXPORT QTextStream &operator<<(QTextStream &s, const QUuid &v);

Q_DECLARE_METATYPE(Shift::StringVector)
Q_DECLARE_METATYPE(QUuid)

#endif

#endif // SBASEPROPERTIES_H
