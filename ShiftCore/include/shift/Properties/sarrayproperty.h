#ifndef SARRAYPROPERTY_H
#define SARRAYPROPERTY_H

#include "shift/sglobal.h"
#include "shift/sdatabase.h"
#include "shift/Properties/spropertycontainer.h"
#include "shift/Properties/sbaseproperties.h"
#include "shift/Serialisation/sloader.h"
#include "Eigen/Core"

namespace Shift
{

// reimplement stream for QTextStream to allow it to work with template classes

template <typename T, typename U> class ArrayProperty : public Property
  {
public:
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

  // called by parent
  static void saveProperty( const Property* p_in, Saver &l); // Mode = Binary / ASCII
  static Property *loadProperty( PropertyContainer* p_in, Loader&); // Mode = Binary / ASCII
  static void AssignFunction(const Property *from, Property *to)
    {
    const U *f = from->castTo<U>();
    U *t = to->castTo<U>();

    xAssert(f && t);
    if(f && t)
      {
      t->mData = f->mData;
      }
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
  };

template <typename T, typename U> void ArrayProperty<T, U>::saveProperty( const Property* p_in, Saver &l)
  {
  Property::saveProperty(p_in, l); // saves the data of the parent class (keeps connections)

  const ArrayProperty* ptr = p_in->castTo<ArrayProperty>(); // cast the input property to an ArrayProperty
  xAssert(ptr);
  if(ptr)
    {
    writeValue(l, ptr->mData);
    }
  }

template <typename T, typename U> Property *ArrayProperty<T, U>::loadProperty( PropertyContainer* p_in, Loader &l)
  {
  Property *prop = Property::loadProperty(p_in, l);

  ArrayProperty* ptr = prop->uncheckedCastTo<ArrayProperty>();
  xAssert(ptr);
  if(ptr)
    {
    readValue(l, ptr->mData);
    }
  return prop;
  }


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
