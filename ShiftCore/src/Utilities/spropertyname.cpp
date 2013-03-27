#include "shift\Utilities\spropertyname.h"

namespace Shift
{

PropertyNameArg::PropertyNameArg()
  {
  _type = TypeEks;
  _data.eks = 0;
  _length = 0;
  }

PropertyNameArg::PropertyNameArg(PropertyNameArg &&oth)
  {
  _type = oth._type;
  _data = oth._data;
  _length = oth._length;
  }

PropertyNameArg& PropertyNameArg::operator=(PropertyNameArg&& oth)
  {
  _type = oth._type;
  _data = oth._data;
  _length = oth._length;

  return *this;
  }

#if X_QT_INTEROP
PropertyNameArg::PropertyNameArg(const QString &t)
  {
  _type = TypeQt;
  _data.qt = &t;
  _length = t.length();
  }
#endif

void PropertyNameArg::toName(PropertyName &out) const
  {
  out.clear();

  if(_type == TypeEks)
    {
    out.resizeAndCopy(_length, _data.eks);
    }
#if X_QT_INTEROP
  else if(_type == TypeQt)
    {
    out = *_data.qt;
    }
#endif
  }

bool PropertyNameArg::isEmpty() const
  {
  return _length == 0;
  }

bool PropertyNameArg::operator==(const PropertyNameArg &oth) const
  {
  if(_type == TypeEks)
    {
    if(oth._type == TypeEks)
      {
      return strcmp(_data.eks, oth._data.eks) == 0;
      }
#if X_QT_INTEROP
    else if(oth._type == TypeQt)
      {
      return  *oth._data.qt == _data.eks;
      }
#endif
    }  
#if X_QT_INTEROP
  else if(_type == TypeQt)
    {
    if(oth._type == TypeEks)
      {
      return *_data.qt == oth._data.eks;
      }
    else if(oth._type == TypeQt)
      {
      return *_data.qt == *oth._data.qt;
      }
    }
#endif

  return false;
  }

bool PropertyNameArg::operator==(const PropertyName &oth) const
  {
  if(_type == TypeEks)
    {
    return oth == _data.eks;
    }
#if X_QT_INTEROP
  else if(_type == TypeQt)
    {
    return (*_data.qt) == oth.data();
    }
#endif

  return false;
  }

}
