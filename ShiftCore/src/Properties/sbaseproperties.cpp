#include "shift/Properties/sbaseproperties.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/Changes/shandler.inl"
#include "shift/sdatabase.h"

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

namespace Utils
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

QTextStream &operator>>(QTextStream &s, SStringVector &v)
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

      Utils::readEscapedQuotedString(s, temp);
      v << Eks::String(temp);
      s.skipWhiteSpace();

      s >> tempCheck;
      }
    }

  return s;
  }

QTextStream &operator<<(QTextStream &s, const SStringVector &v)
  {
  s << "[ ";
  for(xsize i=0, count=v.size(); i<count; ++i)
    {
    Utils::writeEscapedQuotedString(s, v[i]);
    if(i<(count-1))
      {
      s << ", ";
      }
    }
  s << " ]";
  return s;
  }

#define IMPLEMENT_POD_SHIFT_PROPERTY(name) IMPLEMENT_POD_PROPERTY(PODProperty<name>, Shift)

//IMPLEMENT_POD_SHIFT_PROPERTY(bool);

//S_IMPLEMENT_PROPERTY_EXPLICIT(template <typename T>, PODProperty<T>, "POD", Shift)
S_IMPLEMENT_PROPERTY_BASE(PODProperty<bool>, POD, Shift)

template <typename T> const Shift::PropertyInformation *PODProperty<T>::bootstrapStaticTypeInformation(Eks::AllocatorBase *allocator)
  {
  Shift::detail::checkType<PODProperty<T>>();

  Shift::PropertyInformationTyped<PODProperty<T>>::bootstrapTypeInformation(
        &_PODStaticTypeInformation.information,
                                                   "POD",
        PODProperty<T>::ParentType::bootstrapStaticTypeInformation(allocator), allocator);
  return staticTypeInformation();
  }

/*IMPLEMENT_POD_SHIFT_PROPERTY(IntProperty);
IMPLEMENT_POD_SHIFT_PROPERTY(LongIntProperty);
IMPLEMENT_POD_SHIFT_PROPERTY(UnsignedIntProperty);
IMPLEMENT_POD_SHIFT_PROPERTY(LongUnsignedIntProperty);
IMPLEMENT_POD_SHIFT_PROPERTY(FloatProperty);
IMPLEMENT_POD_SHIFT_PROPERTY(DoubleProperty);
IMPLEMENT_POD_SHIFT_PROPERTY(Vector2DProperty);
IMPLEMENT_POD_SHIFT_PROPERTY(Vector3DProperty);
IMPLEMENT_POD_SHIFT_PROPERTY(Vector4DProperty);
IMPLEMENT_POD_SHIFT_PROPERTY(QuaternionProperty);
IMPLEMENT_POD_SHIFT_PROPERTY(StringPropertyBase);
IMPLEMENT_POD_SHIFT_PROPERTY(ColourProperty);
IMPLEMENT_POD_SHIFT_PROPERTY(ByteArrayProperty);
IMPLEMENT_POD_SHIFT_PROPERTY(UuidPropertyBase);

IMPLEMENT_POD_SHIFT_PROPERTY(StringArrayProperty);*/

S_IMPLEMENT_PROPERTY(Data<QUuid>, Shift)

void UuidProperty::createTypeInformation(PropertyInformationTyped<Data<QUuid>> *,
                                           const PropertyInformationCreateData &)
  {
  }

S_IMPLEMENT_PROPERTY(Data<Eks::String>, Shift)

void StringProperty::createTypeInformation(PropertyInformationTyped<Data<Eks::String>> *,
                                           const PropertyInformationCreateData &)
  {
  }

S_IMPLEMENT_PROPERTY(Filename, Shift)

void FilenameProperty::createTypeInformation(PropertyInformationTyped<Filename> *,
                                             const PropertyInformationCreateData &)
  {
  }

namespace detail
{
void assignTo(const Attribute *f, BoolProperty *t)
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

void assignTo(const Attribute *f, IntProperty *t)
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

void assignTo(const Attribute *f, LongIntProperty *t)
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

void assignTo(const Attribute *f, UnsignedIntProperty *t)
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

void assignBetween(const Attribute *f, LongUnsignedIntProperty *t)
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

void FloatProperty::assignBetween(const Attribute *f, Attribute *t)
  {
  FloatProperty *to = t->uncheckedCastTo<FloatProperty>();

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

void DoubleProperty::assignBetween(const Attribute *f, Attribute *t)
  {
  DoubleProperty *to = t->uncheckedCastTo<DoubleProperty>();

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

void Vector2DProperty::assignBetween(const Attribute *f, Attribute *t)
  {
  Vector2DProperty *to = t->uncheckedCastTo<Vector2DProperty>();

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

void Vector3DProperty::assignBetween(const Attribute *f, Attribute *t)
  {
  Vector3DProperty *to = t->uncheckedCastTo<Vector3DProperty>();

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

void Vector4DProperty::assignBetween(const Attribute *f, Attribute *t)
  {
  ColourProperty *to = t->uncheckedCastTo<ColourProperty>();

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

void QuaternionProperty::assignBetween(const Attribute *f, Attribute *t)
  {
  ColourProperty *to = t->uncheckedCastTo<ColourProperty>();

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

void ColourProperty::assignBetween(const Attribute *f, Attribute *t)
  {
  ColourProperty *to = t->uncheckedCastTo<ColourProperty>();

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

void StringPropertyBase::assignBetween(const Attribute *f, Attribute *t)
  {
  StringPropertyBase *to = t->uncheckedCastTo<StringPropertyBase>();

  const StringPropertyBase *sProp = f->castTo<StringPropertyBase>();
  if(sProp)
    {
    to->assign(sProp->value());
    return;
    }
  }

void ByteArrayProperty::assignBetween(const Attribute *, Attribute *)
  {
  xAssertFail();
  }

void UuidPropertyBase::assignBetween(const Attribute *, Attribute *)
  {
  xAssertFail();
  }

void StringArrayProperty::assignBetween(const Attribute *, Attribute *)
  {
  xAssertFail();
  }

}
