#include "shift/Properties/sbaseproperties.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/Changes/shandler.inl"
#include "shift/sdatabase.h"

namespace Shift
{

namespace detail
{
template <typename T> class BasePODPropertyTraits : public PropertyBaseTraits
  {
public:
  static void saveProperty(const Property *p, Saver &l )
    {
    PropertyBaseTraits::saveProperty(p, l);
    }

  static Property *loadProperty(PropertyContainer *parent, Loader &l)
    {
    Property *prop = PropertyBaseTraits::loadProperty(parent, l);
    return prop;
    }

  static bool shouldSavePropertyValue(const Property *)
    {
    return false;
    }

  static void assignProperty(const Shift::Property *p, Shift::Property *l )
    {
    T::assignProperty(p, l);
    }
  };

template <typename T> class PODPropertyTraits : public BasePODPropertyTraits<T>
  {
public:
  static void saveProperty(const Property *p, Saver &l )
    {
    BasePODPropertyTraits<T>::saveProperty(p, l);
    const T *ptr = p->uncheckedCastTo<T>();
    writeValue(l, ptr->_value);
    }

  static Property *loadProperty(PropertyContainer *parent, Loader &l)
    {
    Property *prop = BasePODPropertyTraits<T>::loadProperty(parent, l);
    T *ptr = prop->uncheckedCastTo<T>();
    readValue(l, ptr->_value);
    return prop;
    }

  static bool shouldSavePropertyValue(const Property *p)
    {
    const T *ptr = p->uncheckedCastTo<T>();

    if(BasePODPropertyTraits<T>::shouldSavePropertyValue(p))
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
  };
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
  for(int i=0, count=v.size(); i<count; ++i)
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

#define IMPLEMENT_POD_SHIFT_PROPERTY(name) IMPLEMENT_POD_PROPERTY(name, Shift)

IMPLEMENT_POD_SHIFT_PROPERTY(BoolProperty);
IMPLEMENT_POD_SHIFT_PROPERTY(IntProperty);
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

IMPLEMENT_POD_SHIFT_PROPERTY(StringArrayProperty);

S_IMPLEMENT_PROPERTY(StringProperty, Shift)

void StringProperty::createTypeInformation(PropertyInformationTyped<StringProperty> *,
                                           const PropertyInformationCreateData &)
  {
  }

S_IMPLEMENT_PROPERTY(FilenameProperty, Shift)

void FilenameProperty::createTypeInformation(PropertyInformationTyped<FilenameProperty> *,
                                             const PropertyInformationCreateData &)
  {
  }

void BoolProperty::assignProperty(const Property *f, Property *t)
  {
  BoolProperty *to = t->uncheckedCastTo<BoolProperty>();

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

void IntProperty::assignProperty(const Property *f, Property *t)
  {
  IntProperty *to = t->uncheckedCastTo<IntProperty>();

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

void LongIntProperty::assignProperty(const Property *f, Property *t)
  {
  LongIntProperty *to = t->uncheckedCastTo<LongIntProperty>();

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

void UnsignedIntProperty::assignProperty(const Property *f, Property *t)
  {
  UnsignedIntProperty *to = t->uncheckedCastTo<UnsignedIntProperty>();

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

void LongUnsignedIntProperty::assignProperty(const Property *f, Property *t)
  {
  LongUnsignedIntProperty *to = t->uncheckedCastTo<LongUnsignedIntProperty>();

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

void FloatProperty::assignProperty(const Property *f, Property *t)
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

void DoubleProperty::assignProperty(const Property *f, Property *t)
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

void Vector2DProperty::assignProperty(const Property *f, Property *t)
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

void Vector3DProperty::assignProperty(const Property *f, Property *t)
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

void Vector4DProperty::assignProperty(const Property *f, Property *t)
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

void QuaternionProperty::assignProperty(const Property *f, Property *t)
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

void ColourProperty::assignProperty(const Property *f, Property *t)
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

void StringPropertyBase::assignProperty(const Property *f, Property *t)
  {
  StringPropertyBase *to = t->uncheckedCastTo<StringPropertyBase>();

  const StringPropertyBase *sProp = f->castTo<StringPropertyBase>();
  if(sProp)
    {
    to->assign(sProp->value());
    return;
    }
  }

void ByteArrayProperty::assignProperty(const Property *, Property *)
  {
  xAssertFail();
  }

void StringArrayProperty::assignProperty(const Property *, Property *)
  {
  xAssertFail();
  }

}
