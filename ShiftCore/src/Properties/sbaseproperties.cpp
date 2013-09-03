#include "shift/Properties/sbaseproperties.h"
#include "shift/Properties/sdata.inl"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/Changes/shandler.inl"
#include "shift/sdatabase.h"

namespace Shift
{

namespace detail
{

void readEscapedQuotedString(QTextStream &s, QString &str)
  {
  str.clear();

  while(!s.atEnd())
    {
    QChar tempChar;
    s >> tempChar;
    if(tempChar == '\\')
      {
      s >> tempChar;
      }
    else if(tempChar == '"')
      {
      break;
      }

    str.append(tempChar);
    }
  }

void writeEscapedQuotedString(QTextStream &s, Eks::String str)
  {
  Eks::String::Replacement replacements[] =
  {
    { "\\", "\\\\" },
    { "\"", "\\\"" },
  };

  xAssertFail(); // test the replace fn.
  Eks::String escaped;
  Eks::String::replace(str, &escaped, replacements, X_ARRAY_COUNT(replacements));

  s << "\"" << escaped << "\"";
  }

}
}

QTextStream &operator<<(QTextStream &s, xuint8 v)
  {
  return s << (xuint32)v;
  }

QTextStream &operator>>(QTextStream &s, xuint8 &v)
  {
  xuint32 t;
  s >> t;
  v = (xuint8)t;
  return s;
  }


SHIFT_EXPORT QTextStream &operator>>(QTextStream &s, QUuid &v)
  {
  QString str;
  s >> str;
  v = str;
  return s;
  }

SHIFT_EXPORT QTextStream &operator<<(QTextStream &s, const QUuid &v)
  {
  return s << v.toString();
  }

QTextStream &operator>>(QTextStream &s, Shift::StringVector &v)
  {
  v.clear();
  QString temp;

  s.skipWhiteSpace();

  QChar tempCheck;
  s >> tempCheck;
  if(tempCheck == '[')
    {
    tempCheck = ',';
    while(tempCheck != ']')
      {
      if(tempCheck != ',')
        {
        xAssertFail();
        break;
        }

      s.skipWhiteSpace();
      s >> tempCheck;
      if(tempCheck == ']' || tempCheck != '"')
        {
        break;
        }

      Shift::detail::readEscapedQuotedString(s, temp);
      v << Eks::String(temp);
      s.skipWhiteSpace();

      s >> tempCheck;
      }
    }

  return s;
  }

QTextStream &operator<<(QTextStream &s, const Shift::StringVector &v)
  {
  s << "[ ";
  for(xsize i=0, count=v.size(); i<count; ++i)
    {
    Shift::detail::writeEscapedQuotedString(s, v[i]);
    if(i<(count-1))
      {
      s << ", ";
      }
    }
  s << " ]";
  return s;
  }


namespace Shift
{

namespace detail
{

void getDefault(xuint8 *t)
  {
  *t = 0;
  }

void getDefault(xint32 *t)
  {
  *t = 0;
  }
void getDefault(xint64 *t)
  {
  *t = 0;
  }
void getDefault(xuint32 *t)
  {
  *t = 0;
  }
void getDefault(xuint64 *t)
  {
  *t = 0;
  }
void getDefault(float *t)
  {
  *t = 0.0f;
  }
void getDefault(double *t)
  {
  *t = 0.0;
  }
void getDefault(Eks::Vector2D *t)
  {
  *t = Eks::Vector2D::Zero();
  }
void getDefault(Eks::Vector3D *t)
  {
  *t = Eks::Vector3D::Zero();
  }
void getDefault(Eks::Vector4D *t)
  {
  *t = Eks::Vector4D::Zero();
  }
void getDefault(Eks::Quaternion *t)
  {
  *t = Eks::Quaternion::Identity();
  }
}

#define IMPLEMENT_POD_SHIFT_PROPERTY(T, Mode) IMPLEMENT_POD_PROPERTY(SHIFT_EXPORT, Shift, T, Mode, T)
#define IMPLEMENT_POD_SHIFT_PROPERTY_SPECIAL(T, Mode, name) IMPLEMENT_POD_PROPERTY(SHIFT_EXPORT, Shift, T, Mode, name)

IMPLEMENT_POD_SHIFT_PROPERTY(xuint8, FullData)
IMPLEMENT_POD_SHIFT_PROPERTY(xint32, FullData)
IMPLEMENT_POD_SHIFT_PROPERTY(xint64, FullData)
IMPLEMENT_POD_SHIFT_PROPERTY(xuint32, FullData)
IMPLEMENT_POD_SHIFT_PROPERTY(xuint64, FullData)
IMPLEMENT_POD_SHIFT_PROPERTY(float, FullData)
IMPLEMENT_POD_SHIFT_PROPERTY(double, FullData)
IMPLEMENT_POD_SHIFT_PROPERTY_SPECIAL(Eks::Vector2D, FullData, 2d)
IMPLEMENT_POD_SHIFT_PROPERTY_SPECIAL(Eks::Vector3D, FullData, 3d)
IMPLEMENT_POD_SHIFT_PROPERTY_SPECIAL(Eks::Vector4D, FullData, 4d)
IMPLEMENT_POD_SHIFT_PROPERTY_SPECIAL(Eks::Quaternion, FullData, quat)
IMPLEMENT_POD_SHIFT_PROPERTY_SPECIAL(Eks::String, FullData, str)
IMPLEMENT_POD_SHIFT_PROPERTY_SPECIAL(Eks::Colour, FullData, col)
IMPLEMENT_POD_SHIFT_PROPERTY(QByteArray, FullData)
IMPLEMENT_POD_SHIFT_PROPERTY(QUuid, FullData)
IMPLEMENT_POD_SHIFT_PROPERTY(StringVector, FullData);

namespace detail
{
void podPreGet(const Property *p)
  {
  p->preGet();
  }

void podPreGet(const Attribute *)
  {
  }

void assignTo(const Attribute *, Attribute *)
  {
  xAssertFail();
  }

void assignTo(const Attribute *f, BoolProperty *to)
  {
  const BoolProperty *boolProp = f->castTo<BoolProperty>();
  if(boolProp)
    {
    to->assign(boolProp->value());
    return;
    }

  const IntProperty *intProp = f->castTo<IntProperty>();
  if(intProp)
    {
    to->assign((bool)intProp->value());
    return;
    }

  const LongIntProperty *longIntProp = f->castTo<LongIntProperty>();
  if(longIntProp)
    {
    to->assign(longIntProp->value());
    return;
    }

  const UnsignedIntProperty *uIntProp = f->castTo<UnsignedIntProperty>();
  if(uIntProp)
    {
    to->assign((bool)uIntProp->value());
    return;
    }

  const LongUnsignedIntProperty *uLongIntProp = f->castTo<LongUnsignedIntProperty>();
  if(uLongIntProp)
    {
    to->assign(uLongIntProp->value());
    return;
    }

  const FloatProperty *floatProp = f->castTo<FloatProperty>();
  if(floatProp)
    {
    to->assign(floatProp->value());
    return;
    }

  const DoubleProperty *doubleProp = f->castTo<DoubleProperty>();
  if(doubleProp)
    {
    to->assign(doubleProp->value());
    return;
    }
  }

void assignTo(const Attribute *f, IntProperty *to)
  {
  const BoolProperty *boolProp = f->castTo<BoolProperty>();
  if(boolProp)
    {
    to->assign(boolProp->value());
    return;
    }

  const IntProperty *intProp = f->castTo<IntProperty>();
  if(intProp)
    {
    to->assign(intProp->value());
    return;
    }

  const LongIntProperty *longIntProp = f->castTo<LongIntProperty>();
  if(longIntProp)
    {
    to->assign(longIntProp->value());
    return;
    }

  const UnsignedIntProperty *uIntProp = f->castTo<UnsignedIntProperty>();
  if(uIntProp)
    {
    to->assign(uIntProp->value());
    return;
    }

  const LongUnsignedIntProperty *uLongIntProp = f->castTo<LongUnsignedIntProperty>();
  if(uLongIntProp)
    {
    to->assign(uLongIntProp->value());
    return;
    }

  const FloatProperty *floatProp = f->castTo<FloatProperty>();
  if(floatProp)
    {
    to->assign(floatProp->value());
    return;
    }

  const DoubleProperty *doubleProp = f->castTo<DoubleProperty>();
  if(doubleProp)
    {
    to->assign(doubleProp->value());
    return;
    }
  }

void assignTo(const Attribute *f, LongIntProperty *to)
  {
  const BoolProperty *boolProp = f->castTo<BoolProperty>();
  if(boolProp)
    {
    to->assign(boolProp->value());
    return;
    }

  const IntProperty *intProp = f->castTo<IntProperty>();
  if(intProp)
    {
    to->assign(intProp->value());
    return;
    }

  const LongIntProperty *longIntProp = f->castTo<LongIntProperty>();
  if(longIntProp)
    {
    to->assign(longIntProp->value());
    return;
    }

  const UnsignedIntProperty *uIntProp = f->castTo<UnsignedIntProperty>();
  if(uIntProp)
    {
    to->assign(uIntProp->value());
    return;
    }

  const LongUnsignedIntProperty *uLongIntProp = f->castTo<LongUnsignedIntProperty>();
  if(uLongIntProp)
    {
    to->assign(uLongIntProp->value());
    return;
    }

  const FloatProperty *floatProp = f->castTo<FloatProperty>();
  if(floatProp)
    {
    to->assign(floatProp->value());
    return;
    }

  const DoubleProperty *doubleProp = f->castTo<DoubleProperty>();
  if(doubleProp)
    {
    to->assign(doubleProp->value());
    return;
    }
  }

void assignTo(const Attribute *f, UnsignedIntProperty *to)
  {
  const BoolProperty *boolProp = f->castTo<BoolProperty>();
  if(boolProp)
    {
    to->assign(boolProp->value());
    return;
    }

  const IntProperty *intProp = f->castTo<IntProperty>();
  if(intProp)
    {
    to->assign(intProp->value());
    return;
    }

  const LongIntProperty *longIntProp = f->castTo<LongIntProperty>();
  if(longIntProp)
    {
    to->assign(longIntProp->value());
    return;
    }

  const UnsignedIntProperty *uIntProp = f->castTo<UnsignedIntProperty>();
  if(uIntProp)
    {
    to->assign(uIntProp->value());
    return;
    }

  const LongUnsignedIntProperty *uLongIntProp = f->castTo<LongUnsignedIntProperty>();
  if(uLongIntProp)
    {
    to->assign(uLongIntProp->value());
    return;
    }

  const FloatProperty *floatProp = f->castTo<FloatProperty>();
  if(floatProp)
    {
    to->assign(floatProp->value());
    return;
    }

  const DoubleProperty *doubleProp = f->castTo<DoubleProperty>();
  if(doubleProp)
    {
    to->assign(doubleProp->value());
    return;
    }
  }

void assignTo(const Attribute *f, LongUnsignedIntProperty *to)
  {
  const BoolProperty *boolProp = f->castTo<BoolProperty>();
  if(boolProp)
    {
    to->assign(boolProp->value());
    return;
    }

  const IntProperty *intProp = f->castTo<IntProperty>();
  if(intProp)
    {
    to->assign(intProp->value());
    return;
    }

  const LongIntProperty *longIntProp = f->castTo<LongIntProperty>();
  if(longIntProp)
    {
    to->assign(longIntProp->value());
    return;
    }

  const UnsignedIntProperty *uIntProp = f->castTo<UnsignedIntProperty>();
  if(uIntProp)
    {
    to->assign(uIntProp->value());
    return;
    }

  const LongUnsignedIntProperty *uLongIntProp = f->castTo<LongUnsignedIntProperty>();
  if(uLongIntProp)
    {
    to->assign(uLongIntProp->value());
    return;
    }

  const FloatProperty *floatProp = f->castTo<FloatProperty>();
  if(floatProp)
    {
    to->assign(floatProp->value());
    return;
    }

  const DoubleProperty *doubleProp = f->castTo<DoubleProperty>();
  if(doubleProp)
    {
    to->assign(doubleProp->value());
    return;
    }
  }

void assignTo(const Attribute *f, FloatProperty *to)
  {
  const BoolProperty *boolProp = f->castTo<BoolProperty>();
  if(boolProp)
    {
    to->assign(boolProp->value());
    return;
    }

  const IntProperty *intProp = f->castTo<IntProperty>();
  if(intProp)
    {
    to->assign(intProp->value());
    return;
    }

  const LongIntProperty *longIntProp = f->castTo<LongIntProperty>();
  if(longIntProp)
    {
    to->assign(longIntProp->value());
    return;
    }

  const UnsignedIntProperty *uIntProp = f->castTo<UnsignedIntProperty>();
  if(uIntProp)
    {
    to->assign(uIntProp->value());
    return;
    }

  const LongUnsignedIntProperty *uLongIntProp = f->castTo<LongUnsignedIntProperty>();
  if(uLongIntProp)
    {
    to->assign(uLongIntProp->value());
    return;
    }

  const FloatProperty *floatProp = f->castTo<FloatProperty>();
  if(floatProp)
    {
    to->assign(floatProp->value());
    return;
    }

  const DoubleProperty *doubleProp = f->castTo<DoubleProperty>();
  if(doubleProp)
    {
    to->assign(doubleProp->value());
    return;
    }
  }

void assignTo(const Attribute *f, DoubleProperty *to)
  {
  const BoolProperty *boolProp = f->castTo<BoolProperty>();
  if(boolProp)
    {
    to->assign(boolProp->value());
    return;
    }

  const IntProperty *intProp = f->castTo<IntProperty>();
  if(intProp)
    {
    to->assign(intProp->value());
    return;
    }

  const LongIntProperty *longIntProp = f->castTo<LongIntProperty>();
  if(longIntProp)
    {
    to->assign(longIntProp->value());
    return;
    }

  const UnsignedIntProperty *uIntProp = f->castTo<UnsignedIntProperty>();
  if(uIntProp)
    {
    to->assign(uIntProp->value());
    return;
    }

  const LongUnsignedIntProperty *uLongIntProp = f->castTo<LongUnsignedIntProperty>();
  if(uLongIntProp)
    {
    to->assign(uLongIntProp->value());
    return;
    }

  const FloatProperty *floatProp = f->castTo<FloatProperty>();
  if(floatProp)
    {
    to->assign(floatProp->value());
    return;
    }

  const DoubleProperty *doubleProp = f->castTo<DoubleProperty>();
  if(doubleProp)
    {
    to->assign(doubleProp->value());
    return;
    }
  }

void assignTo(const Attribute *f, Vector2DProperty *to)
  {
  const Vector2DProperty *aProp = f->castTo<Vector2DProperty>();
  if(aProp)
    {
    to->assign(aProp->value());
    return;
    }

  const Vector3DProperty *bProp = f->castTo<Vector3DProperty>();
  if(bProp)
    {
    to->assign(bProp->value().head<2>());
    return;
    }

  const Vector4DProperty *cProp = f->castTo<Vector4DProperty>();
  if(cProp)
    {
    to->assign(cProp->value().head<2>());
    return;
    }

  const ColourProperty *colProp = f->castTo<ColourProperty>();
  if(colProp)
    {
    to->assign(colProp->value().head<2>());
    return;
    }

  const QuaternionProperty *qProp = f->castTo<QuaternionProperty>();
  if(qProp)
    {
    to->assign(qProp->value().coeffs().head<2>());
    return;
    }
  }

void assignTo(const Attribute *f, Vector3DProperty *to)
  {
  const Vector2DProperty *aProp = f->castTo<Vector2DProperty>();
  if(aProp)
    {
    to->assign(aProp->value().head<3>());
    return;
    }

  const Vector3DProperty *bProp = f->castTo<Vector3DProperty>();
  if(bProp)
    {
    to->assign(bProp->value());
    return;
    }

  const Vector4DProperty *cProp = f->castTo<Vector4DProperty>();
  if(cProp)
    {
    to->assign(cProp->value().head<3>());
    return;
    }

  const ColourProperty *colProp = f->castTo<ColourProperty>();
  if(colProp)
    {
    to->assign(colProp->value().head<3>());
    return;
    }

  const QuaternionProperty *qProp = f->castTo<QuaternionProperty>();
  if(qProp)
    {
    to->assign(qProp->value().coeffs().head<3>());
    return;
    }
  }

void assignTo(const Attribute *f, Vector4DProperty *to)
  {
  const Vector2DProperty *aProp = f->castTo<Vector2DProperty>();
  if(aProp)
    {
    Eks::Colour col(aProp->value().head<4>());
    to->assign(col);
    return;
    }

  const Vector3DProperty *bProp = f->castTo<Vector3DProperty>();
  if(bProp)
    {
    Eks::Colour col(bProp->value().head<4>());
    to->assign(col);
    return;
    }

  const Vector4DProperty *cProp = f->castTo<Vector4DProperty>();
  if(cProp)
    {
    to->assign(cProp->value());
    return;
    }

  const ColourProperty *colProp = f->castTo<ColourProperty>();
  if(colProp)
    {
    to->assign(colProp->value());
    return;
    }

  const QuaternionProperty *qProp = f->castTo<QuaternionProperty>();
  if(qProp)
    {
    to->assign(qProp->value().coeffs());
    return;
    }
  }

void assignTo(const Attribute *f, ColourProperty *to)
  {
  const Vector2DProperty *aProp = f->castTo<Vector2DProperty>();
  if(aProp)
    {
    Eks::Colour col(aProp->value().head<4>());
    to->assign(col);
    return;
    }

  const Vector3DProperty *bProp = f->castTo<Vector3DProperty>();
  if(bProp)
    {
    Eks::Colour col(bProp->value().head<4>());
    to->assign(col);
    return;
    }

  const Vector4DProperty *cProp = f->castTo<Vector4DProperty>();
  if(cProp)
    {
    to->assign(cProp->value());
    return;
    }

  const ColourProperty *colProp = f->castTo<ColourProperty>();
  if(colProp)
    {
    to->assign(colProp->value());
    return;
    }

  const QuaternionProperty *qProp = f->castTo<QuaternionProperty>();
  if(qProp)
    {
    Eks::Colour col(qProp->value().coeffs().head<4>());
    to->assign(col);
    return;
    }
  }

void assignTo(const Attribute *f, Data<Eks::String> *to)
  {
  const Data<Eks::String> *sProp = f->castTo<Data<Eks::String>>();
  if(sProp)
    {
    to->assign(sProp->value());
    return;
    }
  }
}
}
