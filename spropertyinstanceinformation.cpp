#include "spropertyinstanceinformation.h"
#include "spropertyinformation.h"
#include "styperegistry.h"
#include "sdatabase.h"

SPropertyInstanceInformation::SPropertyInstanceInformation(bool dynamic)
  {
  _mode = Default;
  _isDynamic = dynamic;
  }

void SPropertyInstanceInformation::destroy(SPropertyInstanceInformation *d)
  {
  xAssert(STypeRegistry::allocator());
  STypeRegistry::allocator()->free(d);
  }

QString g_modeStrings[] = {
  /* Internal      */ "internal",
  /* InputOutput   */ "inputoutput",
  /* InternalInput */ "internalinput",
  /* Input         */ "input",
  /* Output        */ "output",
  /* Computed      */ "computed",
  /* InternalComputed */ "internalcomputed",
  /* UserSettable  */ "usersettable"
};

const QString &SPropertyInstanceInformation::modeString() const
  {
  xAssert(_mode < NumberOfModes);
  return g_modeStrings[_mode];
  }

void SPropertyInstanceInformation::setModeString(const QString &s)
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

void SPropertyInstanceInformation::setInvalidIndex()
  {
  setIndex(X_UINT16_SENTINEL);
  }

SPropertyInstanceInformation::Mode SPropertyInstanceInformation::mode() const
  {
  return Mode(_mode);
  }

void SPropertyInstanceInformation::setMode(Mode m)
  {
  _mode = m;
  }

bool SPropertyInstanceInformation::isDefaultMode() const
  {
  return _mode == Default;
  }

SEmbeddedPropertyInstanceInformation::SEmbeddedPropertyInstanceInformation()
    : SPropertyInstanceInformation(false),
      _holdingTypeInformation(0),
      _location(0),
      _defaultInput(0),
      _compute(0),
      _affects(0),
      _nextSibling(0),
      _isExtraClassMember(false)
  {
  }

SEmbeddedPropertyInstanceInformation *SEmbeddedPropertyInstanceInformation::allocate(xsize size)
  {
  xAssert(STypeRegistry::allocator());
  void *ptr = STypeRegistry::allocator()->alloc(size);

  xAssert(ptr);
  return (SEmbeddedPropertyInstanceInformation*)ptr;
  }

void SEmbeddedPropertyInstanceInformation::initiateProperty(SProperty *propertyToInitiate) const
  {
  if(defaultInput())
    {
    xuint8 *data = (xuint8*)propertyToInitiate;

    const xuint8 *inputPropertyData = data + defaultInput();

    const SProperty *inputProperty = (SProperty*)inputPropertyData;

    xAssert(inputProperty->isDirty());
    xAssert(propertyToInitiate->isDirty());
    inputProperty->connect(propertyToInitiate);
    }
  }

void SEmbeddedPropertyInstanceInformation::setMode(Mode m)
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
    SPropertyInstanceInformation::setMode(m);
    }
  }

void SEmbeddedPropertyInstanceInformation::initiate(const SPropertyInformation *info,
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

void SEmbeddedPropertyInstanceInformation::setCompute(ComputeFunction fn)
  {
  _compute = fn;
  if(_compute)
    {
    _mode = Computed;
    }
  }

void SEmbeddedPropertyInstanceInformation::addAffects(const SEmbeddedPropertyInstanceInformation *info)
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

void SEmbeddedPropertyInstanceInformation::setAffects(const SEmbeddedPropertyInstanceInformation *info)
  {
  xAssert(!_affects);
  xAssert(info);

  addAffects(info);
  }

void SEmbeddedPropertyInstanceInformation::setAffects(const SEmbeddedPropertyInstanceInformation **info, xsize size)
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

void SEmbeddedPropertyInstanceInformation::setAffects(xsize *affects)
  {
  xAssert(!_affects);
  _affects = affects;
  }

const SEmbeddedPropertyInstanceInformation *SEmbeddedPropertyInstanceInformation::resolvePath(const QString &path) const
  {
  SProfileFunction

  const SEmbeddedPropertyInstanceInformation *cur = this;
  const SPropertyInformation *curInfo = cur->childInformation();

  QString name;
  bool escape = false;
  for(int i = 0, s = path.size(); i < s; ++i)
    {
    QChar c = path[i];

    if(c == QChar('\\'))
      {
      escape = true;
      }
    else
      {
      if(!escape && c != SDatabase::pathSeparator())
        {
        name.append(c);
        }

      if(!escape && (c == SDatabase::pathSeparator() || i == (s-1)))
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

void SEmbeddedPropertyInstanceInformation::setDefaultValue(const QString &)
  {
  xAssertFail();
  }

void SEmbeddedPropertyInstanceInformation::setDefaultInput(const SEmbeddedPropertyInstanceInformation *info)
  {
  // find the offset to the holding type information
  xsize targetOffset = 0;
  const SPropertyInformation *targetBase = info->holdingTypeInformation()->findAllocatableBase(targetOffset);
  (void)targetBase;
  // add the instance location
  targetOffset += info->location();

  // find the offset to the holding type information
  xsize sourceOffset = 0;
  const SPropertyInformation *sourceBase = holdingTypeInformation()->findAllocatableBase(sourceOffset);
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

SProperty *SEmbeddedPropertyInstanceInformation::locateProperty(SPropertyContainer *parent) const
  {
  xuint8* parentOffset = reinterpret_cast<xuint8*>(parent);
  xuint8* childOffset = parentOffset + location();
  SProperty *child = reinterpret_cast<SProperty*>(childOffset);

  return child;
  }

const SProperty *SEmbeddedPropertyInstanceInformation::locateProperty(const SPropertyContainer *parent) const
  {
  const xuint8* parentOffset = reinterpret_cast<const xuint8*>(parent);
  const xuint8* childOffset = parentOffset + location();
  const SProperty *child = reinterpret_cast<const SProperty*>(childOffset);
  return child;
  }

const SPropertyContainer *SEmbeddedPropertyInstanceInformation::locateConstParent(const SProperty *prop) const
  {
  return locateParent(const_cast<SProperty*>(prop));
  }

SPropertyContainer *SEmbeddedPropertyInstanceInformation::locateParent(SProperty *prop) const
  {
  xuint8* data = (xuint8*)prop;
  data -= location();

  SPropertyContainer *parent = (SPropertyContainer*)data;
  return parent;
  }

SDynamicPropertyInstanceInformation::SDynamicPropertyInstanceInformation()
    : SPropertyInstanceInformation(true),
      _parent(0),
      _nextSibling(0)
  {
  }
