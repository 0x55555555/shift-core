#ifndef SARRAYPROPERTY_H
#define SARRAYPROPERTY_H

#include "shift/sglobal.h"
#include "shift/sdatabase.h"
#include "shift/Properties/spropertycontainer.h"
#include "shift/Properties/sbaseproperties.h"
#include "shift/TypeInformation/spropertytraits.h"
#include "shift/Serialisation/sloader.h"
#include "Eigen/Core"

namespace Shift
{

namespace detail
{
template <typename T> class ArrayPropertyTraits;
}

template <typename T, typename U> class ArrayProperty : public Property
  {
public:
  typedef detail::ArrayPropertyTraits<U> Traits;
  typedef T ElementType;
  typedef Eigen::Array <T, Eigen::Dynamic, Eigen::Dynamic> EigenArray;

  const EigenArray &data() const { preGet(); return mData; }

  void add(const ArrayProperty <T, U> *in)
    {
    EigenArray result = mData + in->data();
    applyChange(result);
    }

  void add(const ArrayProperty <T, U> *inA, const ArrayProperty <T, U> *inB)
    {
    applyChange(inA->data() + inB->data());
    }

  void resize(xsize width, xsize height)
    {
    EigenArray result = mData;
    result.resize(height, width);

    applyChange(result);
    }

  void resize(xsize size)
    {
    EigenArray result = mData;
    result.resize(1, size);

    applyChange(result);
    }

  xsize size() const
    {
    preGet();
    return mData.cols();
    }

  xsize width() const
    {
    preGet();
    return mData.cols();
    }

  xsize height() const
    {
    preGet();
    return mData.rows();
    }

  const T *get() const
    {
    preGet();
    return mData.data();
    }

  void set(xsize width, xsize height, const Eks::Vector<T> &val)
    {
    EigenArray result = mData;
    result.resize(height, width);

    memcpy(result.data(), &val.front(), sizeof(T)*width*height);

    applyChange(result);
    }

  void setData(const EigenArray &result)
    {
    applyChange(result);
    }

  void setIndex(xsize x, xsize y, const T &val)
    {
    EigenArray result = mData;
    result(y, x) = val;

    applyChange(result);
    }

private:
  class ArrayChange : public Property::DataChange
    {
    S_CHANGE( ArrayChange, Change, Type);
  public:
    ArrayChange(){}
    ArrayChange(const EigenArray &b, const EigenArray &a, Property *prop)
      : Property::DataChange(prop),
      _before(b),
      _after(a)
      {
      }
    const EigenArray &before() const { return _before; }
    const EigenArray &after() const { return _after; }

  private:
    EigenArray _before;
    EigenArray _after;
    bool apply()
      {
      ((U*)property())->mData = after();
      property()->postSet();
      return true;
      }
    bool unApply()
      {
      ((U*)property())->mData = before();
      property()->postSet();
      return true;
      }
    bool inform(bool)
      {
      xAssert(property()->entity());
      property()->entity()->informDirtyObservers(property());
      return true;
      }
    };

  void applyChange(const EigenArray &arr)
    {
    PropertyDoChange(ArrayChange, mData, arr, this);
    }

  Eigen::Array <T, Eigen::Dynamic, Eigen::Dynamic> mData;

  friend class Traits;
  };


class SHIFT_EXPORT FloatArrayProperty : public ArrayProperty<float, FloatArrayProperty>
  {
  S_PROPERTY(FloatArrayProperty, Property, 0)
public:
  };

class SHIFT_EXPORT UIntArrayProperty : public ArrayProperty<xuint32, UIntArrayProperty>
  {
  S_PROPERTY(UIntArrayProperty, Property, 0)
public:
  };

class SHIFT_EXPORT Vector2ArrayProperty : public ArrayProperty<Eks::Vector2D, Vector2ArrayProperty>
  {
  S_PROPERTY(Vector2ArrayProperty, Property, 0)
public:
  };


class SHIFT_EXPORT Vector3ArrayProperty : public ArrayProperty<Eks::Vector3D, Vector3ArrayProperty>
  {
  S_PROPERTY(Vector3ArrayProperty, Property, 0)
public:
  };


class SHIFT_EXPORT Vector4ArrayProperty : public ArrayProperty<Eks::Vector4D, Vector4ArrayProperty>
  {
  S_PROPERTY(Vector4ArrayProperty, Property, 0)
public:
  };

}

S_PROPERTY_INTERFACE(Shift::FloatArrayProperty)
S_PROPERTY_INTERFACE(Shift::UIntArrayProperty)
S_PROPERTY_INTERFACE(Shift::Vector2ArrayProperty)
S_PROPERTY_INTERFACE(Shift::Vector3ArrayProperty)
S_PROPERTY_INTERFACE(Shift::Vector4ArrayProperty)



#endif // SARRAYPROPERTY_H
