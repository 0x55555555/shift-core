#include "shift/Changes/spropertychanges.h"
#include "shift/Properties/scontainerinternaliterators.h"
#include "shift/Properties/sproperty.h"
#include "shift/Properties/scontainer.inl"
#include "shift/sentity.h"
#include "shift/sdatabase.h"
#include "Utilities/XEventLogger.h"

namespace Shift
{

bool Attribute::NameChange::apply()
  {
  SProfileFunction
  attribute()->internalSetName(after(false));
  return true;
  }

bool Attribute::NameChange::unApply()
  {
  SProfileFunction
  attribute()->internalSetName(before(false));
  return true;
  }

bool Attribute::NameChange::inform(bool backwards)
  {
  SProfileFunction
  xAssert(attribute()->entity());
  attribute()->entity()->informTreeObservers(this, backwards );
  return true;
  }


bool Property::ConnectionChange::apply()
  {
  SProfileFunction
  if(_mode == Connect)
    {
    _driver->connectInternal(_driven);
    setParentHasInputConnection(_driven);
    setParentHasOutputConnection(_driver);

    if(!_driver->isUpdating() && !_driven->isUpdating())
      {
      _driver->setDependantsDirty();
      }
    }
  else if(_mode == Disconnect)
    {
    _driver->disconnectInternal(_driven);
    clearParentHasInputConnection(_driven);
    clearParentHasOutputConnection(_driver);
    }
  return true;
  }

bool Property::ConnectionChange::unApply()
  {
  SProfileFunction
  if(_mode == Connect)
    {
    _driver->disconnectInternal(_driven);
    clearParentHasInputConnection(_driven);
    clearParentHasOutputConnection(_driver);
    }
  else if(_mode == Disconnect)
    {
    _driver->connectInternal(_driven);
    setParentHasInputConnection(_driven);
    setParentHasOutputConnection(_driver);

    if(!_driver->isUpdating() && !_driven->isUpdating())
      {
      _driver->setDependantsDirty();
      }
    }
  return true;
  }

bool Property::ConnectionChange::inform(bool back)
  {
  SProfileFunction
  if(_driver->entity())
    {
    _driver->entity()->informConnectionObservers(this, back);
    }
  if(_driven->entity())
    {
    _driven->entity()->informConnectionObservers(this, back);
    }
  return true;
  }

void Property::ConnectionChange::setParentHasInputConnection(Property *prop)
  {
  xAssert(prop);
  prop->_flags.setFlag(Property::ParentHasInput);

  Container *cont = prop->castTo<Container>();
  if(cont)
    {
    xForeach(auto child, LightWalker(cont))
      {
      Property *prop = child->castTo<Property>();
      if(prop && !prop->_flags.hasFlag(Property::ParentHasInput))
        {
        prop->_flags.setFlag(Property::ParentHasInput);
        setParentHasInputConnection(prop);
        }
      }
    }
  }

void Property::ConnectionChange::setParentHasOutputConnection(Property *prop)
  {
  xAssert(prop);
  Container *cont = prop->castTo<Container>();
  if(cont)
    {
    xForeach(auto child, LightWalker(cont))
      {
      Property *prop = child->castTo<Property>();
      if(prop && !prop->_flags.hasFlag(Property::ParentHasOutput))
        {
        prop->_flags.setFlag(Property::ParentHasOutput);
        setParentHasOutputConnection(prop);
        }
      }
    }
  }

void Property::ConnectionChange::clearParentHasInputConnection(Property *prop)
  {
  xAssert(prop);
  Container *cont = prop->castTo<Container>();
  if(cont)
    {
    if(!cont->input() &&
        (cont->isDynamic() ||
         !cont->embeddedBaseInstanceInformation()->isComputed() ) &&
        !cont->_flags.hasFlag(Property::ParentHasInput))
      {
      xForeach(auto child, LightWalker(cont))
        {
        Property *prop = child->castTo<Property>();
        if(prop && prop->_flags.hasFlag(Property::ParentHasInput))
          {
          prop->_flags.clearFlag(Property::ParentHasInput);
          clearParentHasInputConnection(prop);
          }
        }
      }
    }
  }

void Property::ConnectionChange::clearParentHasOutputConnection(Property *prop)
  {
  xAssert(prop);
  Container *cont = prop->castTo<Container>();
  if(cont)
    {
    if(!cont->output() &&
       (cont->isDynamic() ||
        !cont->embeddedBaseInstanceInformation()->affectsSiblings() ) &&
        !cont->_flags.hasFlag(Property::ParentHasOutput))
      {
      xForeach(auto child, LightWalker(cont))
        {
        Property *prop = child->castTo<Property>();
        if(prop && prop->_flags.hasFlag(Property::ParentHasOutput))
          {
          prop->_flags.clearFlag(Property::ParentHasOutput);
          clearParentHasOutputConnection(prop);
          }
        }
      }
    }
  }
ContainerTreeChange::ContainerTreeChange(Container *b, Container *a, Attribute *ent, xsize index)
  : _before(b), _after(a), _attribute(ent), _index(index), _owner(false)
  {
  }

ContainerTreeChange::~ContainerTreeChange()
  {
  if(_owner)
    {
    if(after(false))
      {
      after(false)->database()->deleteDynamicAttribute(_attribute);
      }
    else if(before(false))
      {
      before(false)->database()->deleteDynamicAttribute(_attribute);
      }
    else
      {
      xAssertFailMessage("No parents?");
      }
    }
  }

bool ContainerTreeChange::apply()
  {
  SProfileFunction

  // its possible the tree is computed, but we are trying to insert into it.
  // its also possible this node is part of a ParentHasInput chain.
  // either way, rather than post setting this node, leaving a gaping hole in
  // the dirty chain, we pre get and post set, ensuring dirty flags are correct.

  if(before(false))
    {
    _owner = true;
    before()->preGet();
    before()->internalRemove(property(), false);
    before()->postSet();
    }

  if(after(false))
    {
    _owner = false;
    after()->preGet();
    after()->internalInsert(property(), _index);
    after()->postSet();
    }
  return true;
  }

bool ContainerTreeChange::unApply()
  {
  SProfileFunction
  if(after())
    {
    _owner = true;
    after()->internalRemove(property());
    after()->postSet();
    }

  if(before())
    {
    _owner = false;
    before()->internalInsert(property(), _index);
    before()->postSet();
    }
  return true;
  }

bool ContainerTreeChange::inform(bool back)
  {
  SProfileFunction
  if(after() && (!before() || !before()->isDescendedFrom(after())))
    {
    xAssert(property()->entity());
    property()->entity()->informTreeObservers(this, back);
    }

  if(before() && (!after() || !after()->isDescendedFrom(before())))
    {
    before()->entity()->informTreeObservers(this, back);
    }
  return true;
  }

}
