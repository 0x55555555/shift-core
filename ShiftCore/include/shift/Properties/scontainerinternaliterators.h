#ifndef SCONTAINERINTERNALITERATORS_H
#define SCONTAINERINTERNALITERATORS_H

#include "shift/Properties/scontainer.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/spropertyinstanceinformation.h"

namespace Shift
{

class LightIterator
  {
public:
  LightIterator(Container *cont, xsize embedded, Attribute *dynamic)
      : _cont(cont), _embedded(embedded), _dynamic(dynamic)
    {
    _info = _cont->typeInformation();
    }

  Attribute *operator*()
    {
    if(_embedded != X_SIZE_SENTINEL)
      {
      return _info->childFromIndex(_embedded)->locate(_cont);
      }

    return _dynamic;
    }

  bool operator !=(const LightIterator &it) const
    {
    return _embedded != it._embedded || _dynamic != it._dynamic;
    }

  void operator++()
    {
    if(_embedded != X_SIZE_SENTINEL)
      {
      ++_embedded;
      if(_embedded >= _info->childCount())
        {
        _embedded = X_SIZE_SENTINEL;
        }
      }

    if(_dynamic && _embedded == X_SIZE_SENTINEL)
      {
      const DynamicPropertyInstanceInformation* dynInfo = _dynamic->dynamicInstanceInformation();
      _dynamic = dynInfo->nextSibling();
      }
    }

private:
  Container *_cont;
  const PropertyInformation *_info;
  xsize _embedded;
  Attribute *_dynamic;
  };

class LightWalker
  {
public:
  LightWalker(Container *cont) : _cont(cont)
    {
    }

  LightIterator begin()
    {
    NoUpdateBlock up(_cont);
    xsize firstEmbedded = _cont->typeInformation()->childCount() > 0 ? 0 : X_SIZE_SENTINEL;
    return LightIterator(_cont, firstEmbedded, _cont->firstDynamicChild());
    }

  LightIterator end()
    {
    NoUpdateBlock up(_cont);
    return LightIterator(_cont, X_SIZE_SENTINEL, 0);
    }

private:
  Container *_cont;
  };
}

#endif // SCONTAINERINTERNALITERATORS_H
