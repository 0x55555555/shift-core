#include "spropertyinstanceinformation.h"
#include "spropertyinformation.h"
#include "styperegistry.h"
#include "sdatabase.h"

SPropertyInstanceInformation::SPropertyInstanceInformation(bool dynamic)
  {
  _holdingTypeInformation = 0;
  _compute = 0;
  _location = 0;
  _computeLockedToMainThread = false;
  _affects = 0;
  _mode = Default;
  _isExtraClassMember = false;
  _isDynamic = dynamic;
  _dynamicParent = 0;
  _dynamicNextSibling = 0;
  _nextSibling = 0;
  _defaultInput = 0;
  }

SPropertyInstanceInformation *SPropertyInstanceInformation::allocate(xsize size)
  {
  xAssert(STypeRegistry::allocator());
  void *ptr = STypeRegistry::allocator()->alloc(size);

  xAssert(ptr);
  return (SPropertyInstanceInformation*)ptr;
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

void SPropertyInstanceInformation::setMode(Mode m)
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
    if(!_compute)
      {
      _mode = m;
      }
    }
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

bool SPropertyInstanceInformation::isDefaultMode() const
  {
  return _mode == Default;
  }

void SPropertyInstanceInformation::setCompute(ComputeFunction fn)
  {
  _compute = fn;
  if(_compute)
    {
    _mode = Computed;
    }
  }

void SPropertyInstanceInformation::addAffects(const SPropertyInstanceInformation *info)
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

void SPropertyInstanceInformation::setAffects(const SPropertyInstanceInformation *info)
  {
  xAssert(!_affects);
  xAssert(info);

  addAffects(info);
  }

void SPropertyInstanceInformation::setAffects(const SPropertyInstanceInformation **info, xsize size)
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

void SPropertyInstanceInformation::setAffects(xsize *affects)
  {
  xAssert(!_affects);
  _affects = affects;
  }

void SPropertyInstanceInformation::setDefaultValue(const QString &)
  {
  xAssertFail();
  }

void SPropertyInstanceInformation::setDefaultInput(const SPropertyInstanceInformation *info)
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

  _defaultInput = (xptrdiff)targetOffset - (xptrdiff)sourceOffset;

  xAssert(sourceOffset < sourceBase->size());
  xAssert(targetOffset < sourceBase->size());
  xAssert((sourceOffset + _defaultInput) < sourceBase->size());
  xAssert((targetOffset - _defaultInput) < sourceBase->size());
  xAssert(_defaultInput < (xptrdiff)sourceBase->size());
  }

void SPropertyInstanceInformation::initiateProperty(SProperty *propertyToInitiate) const
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

void SPropertyInstanceInformation::initiate(const SPropertyInformation *info,
                 const QString &name,
                 xsize index,
                 xsize location)
  {
  _childInformation = info;
  _name = name;
  _location = location;
  _index = index;
  }
SProperty *SPropertyInstanceInformation::locateProperty(SPropertyContainer *parent) const
  {
  xuint8* parentOffset = reinterpret_cast<xuint8*>(parent);
  xuint8* childOffset = parentOffset + location();
  SProperty *child = reinterpret_cast<SProperty*>(childOffset);

  return child;
  }

const SProperty *SPropertyInstanceInformation::locateProperty(const SPropertyContainer *parent) const
  {
  const xuint8* parentOffset = reinterpret_cast<const xuint8*>(parent);
  const xuint8* childOffset = parentOffset + location();
  const SProperty *child = reinterpret_cast<const SProperty*>(childOffset);
  return child;
  }

const SPropertyContainer *SPropertyInstanceInformation::locateConstParent(const SProperty *prop) const
  {
  return locateParent(const_cast<SProperty*>(prop));
  }

SPropertyContainer *SPropertyInstanceInformation::locateParent(SProperty *prop) const
  {
  xuint8* data = (xuint8*)prop;
  data -= location();

  SPropertyContainer *parent = (SPropertyContainer*)data;
  return parent;
  }

const SPropertyInstanceInformation *SPropertyInstanceInformation::resolvePath(const QString &path) const
  {
  SProfileFunction

  const SPropertyInstanceInformation *cur = this;
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
