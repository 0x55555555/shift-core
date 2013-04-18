#include "shift/TypeInformation/spropertyinstanceinformation.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/sdatabase.h"

namespace Shift
{

PropertyInstanceInformation::PropertyInstanceInformation(bool dynamic)
  {
  _referenceCount = 0;
  _mode = Default;
  _isDynamic = dynamic;
  }

PropertyInstanceInformation::PropertyInstanceInformation(const PropertyInstanceInformation &c)
  {
  _childInformation = c.childInformation();
  _name = c.name();
  _mode = c.mode();
  _isDynamic = c.isDynamic();

  _referenceCount = 0;
  }

void PropertyInstanceInformation::destroy(
    Eks::AllocatorBase *allocator,
    PropertyInstanceInformation *inst)
  {
  --inst->_referenceCount;
  if(inst->referenceCount() == 0)
    {
    xAssert(allocator);

    const PropertyInformation *info = inst->childInformation();
    bool destroyInfo = info->extendedParent() == inst;

    PropertyInformationFunctions::DestroyInstanceInformationFunction destroy =
      inst->childInformation()->functions().destroyEmbeddedInstanceInformation;

    void *data = destroy(inst);

    if(destroyInfo)
      {
      PropertyInformation::destroyChildren(const_cast<PropertyInformation*>(info), allocator);
      PropertyInformation::destroy(const_cast<PropertyInformation*>(info), allocator);
      }

    allocator->free(data);
    }
  }

const Eks::String &_modeString(xsize i)
  {
  X_HACK(0, "static global", "allocate somewhere nice.")
  static Eks::String g_modeStrings[] = {
    /* Internal      */ "internal",
    /* InputOutput   */ "inputoutput",
    /* InternalInput */ "internalinput",
    /* Input         */ "input",
    /* Output        */ "output",
    /* Computed      */ "computed",
    /* InternalComputed */ "internalcomputed",
    /* UserSettable  */ "usersettable"
  };

  return g_modeStrings[i];
  }

const Eks::String &PropertyInstanceInformation::modeString() const
  {
  xAssert(_mode < NumberOfModes);
  return _modeString(_mode);
  }

void PropertyInstanceInformation::setModeString(const Eks::String &s)
  {
  for(xsize i = 0; i < NumberOfModes; ++i)
    {
    if(_modeString(i) == s)
      {
      _mode = (Mode)i;
      return;
      }
    }

  _mode = Default;
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
      _affectsOwner(false),
      _isExtraClassMember(false)
  {
  }

EmbeddedPropertyInstanceInformation::EmbeddedPropertyInstanceInformation(
    const EmbeddedPropertyInstanceInformation &o)
    : PropertyInstanceInformation(o),
    _holdingTypeInformation(o.holdingTypeInformation()),
    _location(o.location()),
    _defaultInput(o.defaultInput()),
    _compute(o.compute()),
    _affects(o.affects()),
    _affectsOwner(false),
    _isExtraClassMember(false),
    _index(o._index)
  {
  xAssert(!o.isExtraClassMember())
  }

EmbeddedPropertyInstanceInformation *EmbeddedPropertyInstanceInformation::allocate(
    Eks::AllocatorBase *allocator,
    const Eks::ResourceDescription &fmt)
  {
  xAssert(allocator);
  void *ptr = allocator->alloc(fmt);

  xAssert(ptr);
  return (EmbeddedPropertyInstanceInformation*)ptr;
  }

void EmbeddedPropertyInstanceInformation::destroy(
    Eks::AllocatorBase *allocator,
    EmbeddedPropertyInstanceInformation *inst)
  {
  if(inst->referenceCount() == 1 && inst->affectsOwner())
    {
    allocator->free(inst->affects());
    }

  PropertyInstanceInformation::destroy(allocator, inst);
  }

void EmbeddedPropertyInstanceInformation::initiateAttribute(Attribute *propertyToInitiate) const
  {
  if(defaultInput())
    {
    Property *thsProp = propertyToInitiate->uncheckedCastTo<Property>();
    xuint8 *data = (xuint8*)propertyToInitiate;

    const xuint8 *inputPropertyData = data + defaultInput();

    const Property *inputProperty = ((Attribute*)inputPropertyData)->uncheckedCastTo<Property>();

    xAssert(inputProperty->isDirty());
    xAssert(!propertyToInitiate->castTo<Property>() || propertyToInitiate->uncheckedCastTo<Property>()->isDirty());
    inputProperty->connect(thsProp);
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
                 const NameArg &n,
                 xsize index,
                 xsize location)
  {
  setChildInformation(info);
  n.toName(name());

  xAssert(location < X_UINT16_SENTINEL);
  setLocation((xuint16)location);

  xAssert(index < X_UINT8_SENTINEL);
  setIndex((xuint8)index);
  }

void EmbeddedPropertyInstanceInformation::setCompute(ComputeFunction fn)
  {
  _compute = fn;
  if(_compute)
    {
    _mode = Computed;
    }
  }

void EmbeddedPropertyInstanceInformation::setAffects(
    const PropertyInformationCreateData &data,
    const EmbeddedPropertyInstanceInformation *info)
  {
  xAssert(!_affects);
  xAssert(info);

  setAffects(data, &info, 1);
  }

void EmbeddedPropertyInstanceInformation::setAffects(
    const PropertyInformationCreateData &data,
    const EmbeddedPropertyInstanceInformation **info,
    xsize size)
  {
  setAffects(PropertyInformationChildrenCreator::createAffects(data.allocator, info, size), true);
  }

void EmbeddedPropertyInstanceInformation::setAffects(xsize *affects, bool affectsOwner)
  {
  xAssert(!_affects || !_affectsOwner);
  xAssert(affects);

  _affects = affects;
  _affectsOwner = affectsOwner;
  }

const EmbeddedPropertyInstanceInformation *EmbeddedPropertyInstanceInformation::resolvePath(const Eks::String &path) const
  {
  SProfileFunction

  const EmbeddedPropertyInstanceInformation *cur = this;
  const PropertyInformation *curInfo = cur->childInformation();

  Eks::String name;
  bool escape = false;
  for(xsize i = 0, s = path.size(); i < s; ++i)
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
  xAssert(info->childInformation()->inheritsFromType(Property::staticTypeInformation()));
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
  xAssert(sourceBase->inheritsFromType(targetBase));

  xptrdiff inp = (xptrdiff)targetOffset - (xptrdiff)sourceOffset;
  xAssert(inp < X_INT16_MAX && inp > X_INT16_MIN)
  _defaultInput = inp;

  xAssert(sourceOffset < sourceBase->size());
  xAssert(targetOffset < sourceBase->size());
  xAssert((sourceOffset + _defaultInput) < sourceBase->size());
  xAssert((targetOffset - _defaultInput) < sourceBase->size());
  xAssert(_defaultInput < (xptrdiff)sourceBase->size());
  }

Attribute *EmbeddedPropertyInstanceInformation::locate(Container *parent) const
  {
  xuint8* parentOffset = reinterpret_cast<xuint8*>(parent);
  xuint8* childOffset = parentOffset + location();
  Attribute *child = reinterpret_cast<Attribute*>(childOffset);

  return child;
  }

const Attribute *EmbeddedPropertyInstanceInformation::locate(const Container *parent) const
  {
  const xuint8* parentOffset = reinterpret_cast<const xuint8*>(parent);
  const xuint8* childOffset = parentOffset + location();
  const Attribute *child = reinterpret_cast<const Attribute*>(childOffset);
  return child;
  }

const Container *EmbeddedPropertyInstanceInformation::locateConstParent(const Attribute *prop) const
  {
  return locateParent(const_cast<Attribute*>(prop));
  }

Container *EmbeddedPropertyInstanceInformation::locateParent(Attribute *prop) const
  {
  xuint8* data = (xuint8*)prop;
  data -= location();

  Container *parent = (Container*)data;
  return parent;
  }

DynamicPropertyInstanceInformation::DynamicPropertyInstanceInformation()
    : PropertyInstanceInformation(true),
      _parent(0),
      _nextSibling(0)
  {
  }
DynamicPropertyInstanceInformation::DynamicPropertyInstanceInformation(
    const DynamicPropertyInstanceInformation &o)
    : PropertyInstanceInformation(o),
    _parent(0),
    _nextSibling(0),
    _index(o._index)
  {
  }

void DynamicPropertyInstanceInformation::setInvalidIndex()
  {
  setIndex(X_SIZE_SENTINEL);
  }

}
