#include "shift/TypeInformation/spropertyinstanceinformation.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/sdatabase.h"

namespace Shift
{

PropertyInstanceInformation::PropertyInstanceInformation(bool dynamic)
  {
  _mode = Default;
  _isDynamic = dynamic;
  }

void PropertyInstanceInformation::destroy(PropertyInstanceInformation *d)
  {
  xAssert(TypeRegistry::allocator());
  TypeRegistry::allocator()->free(d);
  }

Eks::String g_modeStrings[] = {
  /* Internal      */ "internal",
  /* InputOutput   */ "inputoutput",
  /* InternalInput */ "internalinput",
  /* Input         */ "input",
  /* Output        */ "output",
  /* Computed      */ "computed",
  /* InternalComputed */ "internalcomputed",
  /* UserSettable  */ "usersettable"
};

const Eks::String &PropertyInstanceInformation::modeString() const
  {
  xAssert(_mode < NumberOfModes);
  return g_modeStrings[_mode];
  }

void PropertyInstanceInformation::setModeString(const Eks::String &s)
  {
  for(xsize i = 0; i < NumberOfModes; ++i)
    {
    if(g_modeStrings[i] == s)
      {
      _mode = (Mode)i;
      return;
      }
    }

  _mode = Default;
  }

void PropertyInstanceInformation::setInvalidIndex()
  {
  setIndex(X_UINT16_SENTINEL);
  }

PropertyInstanceInformation::Mode PropertyInstanceInformation::mode() const
  {
  return Mode(_mode);
  }

void PropertyInstanceInformation::setMode(Mode m)
  {
  _mode = m;
  }

bool PropertyInstanceInformation::isDefaultMode() const
  {
  return _mode == Default;
  }

EmbeddedPropertyInstanceInformation::EmbeddedPropertyInstanceInformation()
    : PropertyInstanceInformation(false),
      _holdingTypeInformation(0),
      _location(0),
      _defaultInput(0),
      _compute(0),
      _affects(0),
      _isExtraClassMember(false)
  {
  }

EmbeddedPropertyInstanceInformation *EmbeddedPropertyInstanceInformation::allocate(xsize size)
  {
  xAssert(TypeRegistry::allocator());
  void *ptr = TypeRegistry::allocator()->alloc(size);

  xAssert(ptr);
  return (EmbeddedPropertyInstanceInformation*)ptr;
  }

void EmbeddedPropertyInstanceInformation::initiateProperty(Property *propertyToInitiate) const
  {
  if(defaultInput())
    {
    xuint8 *data = (xuint8*)propertyToInitiate;

    const xuint8 *inputPropertyData = data + defaultInput();

    const Property *inputProperty = (Property*)inputPropertyData;

    xAssert(inputProperty->isDirty());
    xAssert(propertyToInitiate->isDirty());
    inputProperty->connect(propertyToInitiate);
    }
  }

void EmbeddedPropertyInstanceInformation::setMode(Mode m)
  {
  if(_compute)
    {
    if(m == Internal)
      {
      _mode = InternalComputed;
      }
    else
      {
      xAssertFail();
      }
    }
  else
    {
    PropertyInstanceInformation::setMode(m);
    }
  }

void EmbeddedPropertyInstanceInformation::initiate(const PropertyInformation *info,
                 const QString &n,
                 xsize index,
                 xsize location)
  {
  setChildInformation(info);
  name() = n;
  xAssert(location < X_UINT16_SENTINEL);
  setLocation(location);
  xAssert(index < X_UINT16_SENTINEL);
  setIndex(index);
  }

void EmbeddedPropertyInstanceInformation::setCompute(ComputeFunction fn)
  {
  _compute = fn;
  if(_compute)
    {
    _mode = Computed;
    }
  }

void EmbeddedPropertyInstanceInformation::addAffects(const EmbeddedPropertyInstanceInformation *info)
  {
  xsize *oldAffects = _affects;
  xsize affectsSize = 0;
  if(oldAffects)
    {
    xAssert(info);

    xsize *current = _affects;
    while(*current)
      {
      current++;
      affectsSize++;
      }
    }

  _affects = new xsize[affectsSize+2]; // one for the new one, one for the end 0

  if(oldAffects)
    {
    memcpy(_affects, oldAffects, sizeof(xsize)*affectsSize);
    delete oldAffects;
    }

  _affects[affectsSize] = info->location();
  _affects[affectsSize+1] = 0;
  }

void EmbeddedPropertyInstanceInformation::setAffects(const EmbeddedPropertyInstanceInformation *info)
  {
  xAssert(!_affects);
  xAssert(info);

  addAffects(info);
  }

void EmbeddedPropertyInstanceInformation::setAffects(const EmbeddedPropertyInstanceInformation **info, xsize size)
  {
  xAssert(!_affects);
  xAssert(info);

  _affects = new xsize[size+1];

  for(xsize i = 0; i < size; ++i)
    {
    _affects[i] = info[i]->location();
    }

  _affects[size] = 0;
  }

void EmbeddedPropertyInstanceInformation::setAffects(xsize *affects)
  {
  xAssert(!_affects);
  _affects = affects;
  }

const EmbeddedPropertyInstanceInformation *EmbeddedPropertyInstanceInformation::resolvePath(const Eks::String &path) const
  {
  SProfileFunction

  const EmbeddedPropertyInstanceInformation *cur = this;
  const PropertyInformation *curInfo = cur->childInformation();

  Eks::String name;
  bool escape = false;
  for(int i = 0, s = path.size(); i < s; ++i)
    {
    Eks::Char c = path[i];

    if(c == Eks::Char('\\'))
      {
      escape = true;
      }
    else
      {
      if(!escape && c != Database::pathSeparator()[0])
        {
        name.append(c);
        }

      if(!escape && (c == Database::pathSeparator()[0] || i == (s-1)))
        {
        if(name == "..")
          {
          xAssert(cur);
          if(!cur)
            {
            return 0;
            }

          curInfo = cur->holdingTypeInformation();
          if(!curInfo)
            {
            return 0;
            }

          cur = curInfo->extendedParent();
          }
        else
          {
          xAssert(curInfo);
          cur = curInfo->childFromName(name);
          if(!cur)
            {
            return 0;
            }

          curInfo = cur->childInformation();
          }

        if(!cur && !curInfo)
          {
          return 0;
          }
        xAssert(curInfo);

        name.clear();
        }
      escape = false;
      }
    }
  return cur;
  }

void EmbeddedPropertyInstanceInformation::setDefaultValue(const QString &)
  {
  xAssertFail();
  }

void EmbeddedPropertyInstanceInformation::setDefaultInput(const EmbeddedPropertyInstanceInformation *info)
  {
  // find the offset to the holding type information
  xsize targetOffset = 0;
  const PropertyInformation *targetBase = info->holdingTypeInformation()->findAllocatableBase(targetOffset);
  (void)targetBase;
  // add the instance location
  targetOffset += info->location();

  // find the offset to the holding type information
  xsize sourceOffset = 0;
  const PropertyInformation *sourceBase = holdingTypeInformation()->findAllocatableBase(sourceOffset);
  (void)sourceBase;
  // add the instance location
  sourceOffset += location();

  // cannot add a default input between to separate allocatable types.
  xAssert(targetBase == sourceBase);

  xptrdiff inp = (xptrdiff)targetOffset - (xptrdiff)sourceOffset;
  xAssert(inp < X_INT16_MAX && inp > X_INT16_MIN)
  _defaultInput = inp;

  xAssert(sourceOffset < sourceBase->size());
  xAssert(targetOffset < sourceBase->size());
  xAssert((sourceOffset + _defaultInput) < sourceBase->size());
  xAssert((targetOffset - _defaultInput) < sourceBase->size());
  xAssert(_defaultInput < (xptrdiff)sourceBase->size());
  }

Property *EmbeddedPropertyInstanceInformation::locateProperty(PropertyContainer *parent) const
  {
  xuint8* parentOffset = reinterpret_cast<xuint8*>(parent);
  xuint8* childOffset = parentOffset + location();
  Property *child = reinterpret_cast<Property*>(childOffset);

  return child;
  }

const Property *EmbeddedPropertyInstanceInformation::locateProperty(const PropertyContainer *parent) const
  {
  const xuint8* parentOffset = reinterpret_cast<const xuint8*>(parent);
  const xuint8* childOffset = parentOffset + location();
  const Property *child = reinterpret_cast<const Property*>(childOffset);
  return child;
  }

const PropertyContainer *EmbeddedPropertyInstanceInformation::locateConstParent(const Property *prop) const
  {
  return locateParent(const_cast<Property*>(prop));
  }

PropertyContainer *EmbeddedPropertyInstanceInformation::locateParent(Property *prop) const
  {
  xuint8* data = (xuint8*)prop;
  data -= location();

  PropertyContainer *parent = (PropertyContainer*)data;
  return parent;
  }

DynamicPropertyInstanceInformation::DynamicPropertyInstanceInformation()
    : PropertyInstanceInformation(true),
      _parent(0),
      _nextSibling(0)
  {
  }

}
