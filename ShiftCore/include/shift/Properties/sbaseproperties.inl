#ifndef SBASEPROPERTIES_INL
#define SBASEPROPERTIES_INL


#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "XTemporaryAllocator"
#include "shift/Changes/spropertychanges.h"
#include "shift/TypeInformation/spropertytraits.h"
#include "shift/TypeInformation/spropertygroup.h"

#if X_QT_INTEROP

Q_DECLARE_METATYPE(SStringVector)

SHIFT_EXPORT QTextStream &operator<<(QTextStream &s, xuint8 v);
SHIFT_EXPORT QTextStream &operator>>(QTextStream &s, xuint8 &v);

SHIFT_EXPORT QTextStream &operator>>(QTextStream &s, SStringVector &);
SHIFT_EXPORT QTextStream &operator<<(QTextStream &s, const SStringVector &v);

SHIFT_EXPORT QTextStream &operator>>(QTextStream &s, QUuid &);
SHIFT_EXPORT QTextStream &operator<<(QTextStream &s, const QUuid &v);

#endif

namespace Shift
{

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
void assignTo(const Attribute *, Attribute *);

void podPreGet(const Property *p)
  {
  p->preGet();
  }

void podPreGet(const Attribute *)
  {
  }

template <typename T, int IsAttribute, int IsFull> class PODPropertyTraits
  : public PropertyBaseTraits
  {
public:
  static bool shouldSaveValue(const Attribute *)
    {
    return false;
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

template <typename T, DataMode Mode> class DataInformation
  {
public:
  static PropertyGroup::Information _staticTypeInformation;
  };

template <typename T, DataMode Mode>
  PropertyGroup::Information DataInformation<T, Mode>::_staticTypeInformation =
      Shift::propertyGroup().registerPropertyInformation(&Data<T, Mode>::_staticTypeInformation,
                                                         Data<T, Mode>::bootstrapStaticTypeInformation);

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
        &DataInformation<T, Mode>::_staticTypeInformation.information,
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

#endif // SBASEPROPERTIES_INL
