#include "shift/Properties/sproperty.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/Changes/schange.h"
#include "shift/Changes/shandler.inl"
#include "shift/Changes/spropertychanges.h"
#include "shift/Utilities/satomichelper.h"
#include "XProfiler"
#include "XConvertScriptSTL.h"
#include <QtConcurrent/QtConcurrentRun>

namespace Shift
{

xCompileTimeAssert(sizeof(Property) <= (sizeof(Attribute) + sizeof(void*) * 4));
S_IMPLEMENT_PROPERTY(Property, Shift)


void Property::createTypeInformation(PropertyInformationTyped<Property> *info,
                                      const PropertyInformationCreateData &data)
  {
  if(data.registerInterfaces)
    {
    auto *api = info->apiInterface();

    typedef XScript::XMethodToGetter<Property, Eks::Vector<Property*> (), &Property::affects> AffectsGetter;

    static XScript::ClassDef<0,4,0> cls = {
      {
        api->property<Property *, const Property *, &Property::input, &Property::setInput>("input"),

        api->property<Property *, &Property::output>("firstOutput"),
        api->property<Property *, &Property::nextOutput>("nextOutput"),

        api->property<AffectsGetter>("affects")
      }
    };

    api->buildInterface(cls);
    }
  }

void Property::setDependantsDirty()
  {
  for(Property *o=output(); o; o = o->nextOutput())
    {
    xAssert(o != o->nextOutput());
    o->setDirty();
    }

  const PropertyInstanceInformation *childBase = baseInstanceInformation();
  if(!isDynamic())
    {
    const EmbeddedPropertyInstanceInformation *child = childBase->embeddedInfo();
    const xsize *affectsLocations = child->affects();
    if(affectsLocations)
      {
      xuint8* parentLocation = (xuint8*)this;
      parentLocation -= child->location();

      for(;*affectsLocations; ++affectsLocations)
        {
        xuint8* affectedLocation = parentLocation + *affectsLocations;
        Property *affectsProp = (Property *)affectedLocation;

        xAssert(affectsProp);
        affectsProp->setDirty();
        }
      }
    }

  // if there is an input, a parent input or this is computed,
  // then when we are dirtied we need to dirty our children
  if(input() || _flags.hasFlag(Property::ParentHasInput) || isComputed())
    {
    Container *c = castTo<Container>();
    if(c)
      {
      const PropertyInformation *info = c->typeInformation();
      for(xsize i = 0, s = info->childCount(); i < s; ++i)
        {
        const EmbeddedPropertyInstanceInformation *inst = info->childFromIndex(i);
        Attribute *child = inst->locate(c);

        if(Property *childProp = child->castTo<Property>())
          {
          childProp->setDirty();
          }
        }
      }
    }

  // if we know the parent has an output
  if(_flags.hasFlag(Property::ParentHasOutput))
    {
    Property *parentProp = parent();

    if(_dirty)
      {
      while(parentProp &&
            (parentProp->_flags.hasFlag(Property::ParentHasOutput) || parentProp->output()) &&
            !parentProp->isUpdating())
        {
        parentProp->setDirty();

        parentProp = parentProp->parent();
        }
      }

    while(parentProp &&
          (parentProp->_flags.hasFlag(Property::ParentHasOutput) || parentProp->output()))
      {
      parentProp->setDependantsDirty();

      parentProp = parentProp->parent();
      }
    }
  }

Property::Property()
  : _input(0), _output(0), _nextOutput(0), _updating(false), _flags(0), _dirty(true)
  {
  }

void Property::setInput(const Property *inp)
  {
  SProfileFunction
  if(inp)
    {
    PropertyDoChange(ConnectionChange, ConnectionChange::Connect, (Property*)inp, this);
    }
  else if(input())
    {
    PropertyDoChange(ConnectionChange, ConnectionChange::Disconnect, (Property*)input(), this);
    }
  }

void Property::connect(Property *prop) const
  {
  SProfileFunction
  if(prop && prop != this)
    {
    prop->setInput(this);
    }
  else
    {
    xAssertFail();
    }
  }

void Property::connect(const Eks::Vector<Property*> &l) const
  {
  if(l.size())
    {
    Block b(l.front()->handler());
    Q_FOREACH(Property *p, l)
      {
      connect(p);
      }
    }
  }

void Property::disconnect(Property *prop) const
  {
  xAssert(this == prop->input());
  prop->setInput(0);
  }

bool Property::isComputed() const
  {
  if(!isDynamic())
    {
    const EmbeddedPropertyInstanceInformation *staticInfo = embeddedBaseInstanceInformation();
    return staticInfo->compute() != 0;
    }
  return false;
  }

void Property::disconnect() const
  {
  SProfileFunction
  if(_input)
    {
    ((Property*)_input)->disconnect((Property*)this);
    }

  while(_output)
    {
    disconnect(_output);
    }
  }

Eks::Vector<const Property *> Property::affects() const
  {
  Eks::Vector<Property *> aff = const_cast<Property*>(this)->affects();
  union
    {
    Eks::Vector<Property *>* in;
    Eks::Vector<const Property*>* out;
    } conv;

  conv.in = &aff;
  return *conv.out;
  }

Eks::Vector<Property *> Property::affects()
  {
  Eks::Vector<Property *> ret;

  const EmbeddedPropertyInstanceInformation *info = embeddedBaseInstanceInformation();
  if(!info)
    {
    return ret;
    }

  xsize *affects = info->affects();
  if(!affects)
    {
    return ret;
    }

  Container *par = parent();
  const PropertyInformation *parentInfo = par->typeInformation();
  while(*affects)
    {
    const EmbeddedPropertyInstanceInformation *affected = parentInfo->child(*affects);

    Attribute *attr = affected->locate(par);
    ret << attr->uncheckedCastTo<Property>();
    affects++;
    }

  return ret;
  }
void Property::connectInternal(Property *prop) const
  {
  // prop can't already have an output
  if(prop->hasInput())
    {
    return;
    }
  prop->_input = (Property*)this;

  Property **output = (Property**)&_output;
  while(*output)
    {
    Property **nextOp = &((*output)->_nextOutput);

    xAssert(*nextOp != prop);
    output = nextOp;
    }

  if(output)
    {
    *output = prop;
    }

#ifdef X_DEBUG
  const Property *p = this;
  while(p)
    {
    xAssert(p != p->nextOutput());
    p = p->nextOutput();
    }
#endif
  }

void Property::disconnectInternal(Property *prop) const
  {
  xAssert(prop->_input == this);

  prop->_input = 0;

  Property **output = (Property**)&_output;
  while(*output)
    {
    xAssert(*output != (*output)->_nextOutput);

    if((*output) == prop)
      {
      (*output) = (*output)->_nextOutput;
      break;
      }

    output = &((*output)->_nextOutput);
    }

  prop->_nextOutput = 0;

#ifdef X_DEBUG
  const Property *p = this;
  while(p)
    {
    xAssert(p != p->nextOutput());
    p = p->nextOutput();
    }
#endif
  }

void Property::postSet()
  {
  SProfileFunction
  if(!_updating)
    {
    setDependantsDirty();
    }

  _dirty = false;
  }

void Property::setDirty()
  {
  SProfileFunction

  if(!_dirty)
    {
    _dirty = true;

    setDependantsDirty();
    xAssert(isDirty());

#ifdef S_CENTRAL_CHANGE_HANDLER
    Entity *ent = entity();
    if(ent)
      {
      ent->informDirtyObservers(this);
      }
#endif
    }
  }

void Property::updateParent() const
  {
  const Property *par = parent();
  if(par)
    {
    par->preGet();
    }
  }

bool Property::beginUpdate() const
  {
  // this is a const function, but because we delay computation we may need to assign here
  Property *prop = const_cast<Property*>(this);

  bool success = AtomicHelper::trySet(&prop->_updating, 1, 0);

  if(!success)
    {
    return false;
    }

  prop->_dirty = false;
  return true;
  }

void Property::endUpdate() const
  {
  Property *prop = const_cast<Property*>(this);
  bool success = AtomicHelper::trySet(&prop->_updating, 0, 1);
  xAssert(success);
  }

void Property::update() const
  {
  SProfileFunction
  StateStorageBlock ss(false, const_cast<Handler*>(handler()));

  if(!beginUpdate())
    {
    return;
    }

  // if the parent is computed or has input, we need to update it,
  // which may update us. Note that we should be made dirty recursively
  // when the parent is dirtied, so its safe to do this in update not preGet.
  if(_flags.hasFlag(ParentHasInput))
    {
    updateParent();
    }

  concurrentUpdate();

  // dirty can be set again in compute functions.
  xAssert(!isDirty());
  }

void Property::concurrentUpdate() const
  {
  Property *prop = const_cast<Property*>(this);

  if(!isDynamic())
    {
    const EmbeddedInstanceInformation *child = embeddedBaseInstanceInformation();
    EmbeddedInstanceInformation::ComputeFunction compute = child->compute();
    if(compute)
      {
      Container *par = prop->embeddedParent();
      xAssert(par);

      compute(child, par);
      }
    else if(input())
      {
      prop->assign(input());
      }
    }
  else if(input())
    {
    prop->assign(input());
    }

  endUpdate();
  }

void Property::concurrentPreGet() const
  {
  if(!isDirty() || !beginUpdate())
    {
    return;
    }

  const Container *par = parent();
  const PropertyInformation *info = par->typeInformation();
  xForeach(const EmbeddedInstanceInformation *inst, info->childWalker())
    {
    auto walker = inst->affectsWalker(parent());
    xForeach(const Property *affectsProp, walker)
      {
      if(affectsProp == this)
        {
        affectsProp->preGet();
        }
      }
    }

  if(_flags.hasFlag(ParentHasInput))
    {
    updateParent();
    }

  QtConcurrent::run(this, &Property::concurrentUpdate);
  }

NoUpdateBlock::NoUpdateBlock(Attribute *p) : _prop(p->castTo<Property>())
  {
  if(_prop)
    {
    _oldDirty = _prop->isDirty();
    _prop->beginUpdate();
    }
  }

NoUpdateBlock::~NoUpdateBlock()
  {
  if(_prop)
    {
    _prop->endUpdate();
    if(_oldDirty)
      {
      _prop->_dirty = true;
      }
    }
  }

}
