#ifndef SDATA_H
#define SDATA_H

#include "shift/Properties/sproperty.h"

namespace Shift
{

namespace detail
{
template <typename T> class PODChange;
template <typename T> class PODComputeChange;
template <typename T> class PODLock;
template <typename T> class PODComputeLock;
template <typename T, int IsAttribute, int IsFull> class PODPropertyTraits;
template <typename T> class DataEmbeddedInstanceInformation;

SHIFT_EXPORT void podPreGet(const Property *);
SHIFT_EXPORT void podPreGet(const Attribute *);
}

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
  typedef detail::DataEmbeddedInstanceInformation<PODPropertyType> EmbeddedInstanceInformation;
  typedef typename ParentType::DynamicInstanceInformation DynamicInstanceInformation;

  S_PROPERTY(PODPropertyType, ParentType);

public:
  typedef T PODType;

  typedef detail::PODChange<PODPropertyType> Change;
  typedef detail::PODComputeChange<PODPropertyType> ComputeChange;

  PODPropertyType &operator=(const PODType &p)
    {
    assign(p);
    return *this;
    }

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

  typedef detail::PODLock<PODPropertyType> Lock;
  typedef detail::PODComputeLock<PODPropertyType> ComputeLock;

  inline ComputeLock computeLock()
    {
    return this;
    }

protected:
  XPropertyMember(T, value);

  friend class ComputeLock;
  friend class ComputeChange;
  friend class Change;
  friend class Traits;
  friend class Lock;
  friend class EmbeddedInstanceInformation;

private:
  static Shift::PropertyInformation **staticTypeInformationInternal();
  };

}
#endif // SDATA_H
