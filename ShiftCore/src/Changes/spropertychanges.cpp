#include "shift/Changes/spropertychanges.h"
#include "shift/Properties/scontaineriterators.h"
#include "shift/Properties/sproperty.h"
#include "shift/sentity.h"

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
    //if(_driven->typeInformation()->inheritsFromType(_driver->typeInformation()))
      {
      setParentHasInputConnection(_driven);
      setParentHasOutputConnection(_driver);
      }

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
    //if(_driven->typeInformation()->inheritsFromType(_driver->typeInformation()))
      {
      setParentHasInputConnection(_driven);
      setParentHasOutputConnection(_driver);
      }

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
    xForeach(auto child, cont->walker<Property>())
      {
      if(!child->_flags.hasFlag(Property::ParentHasInput))
        {
        child->_flags.setFlag(Property::ParentHasInput);
        setParentHasInputConnection(child);
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
    xForeach(auto child, cont->walker<Property>())
      {
      if(!child->_flags.hasFlag(Property::ParentHasOutput))
        {
        child->_flags.setFlag(Property::ParentHasOutput);
        setParentHasOutputConnection(child);
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
      xForeach(auto child, cont->walker<Property>())
        {
        if(child->_flags.hasFlag(Property::ParentHasInput))
          {
          child->_flags.clearFlag(Property::ParentHasInput);
          clearParentHasInputConnection(child);
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
      xForeach(auto child, cont->walker<Property>())
        {
        if(child->_flags.hasFlag(Property::ParentHasOutput))
          {
          child->_flags.clearFlag(Property::ParentHasOutput);
          clearParentHasOutputConnection(child);
          }
        }
      }
    }
  }

}
