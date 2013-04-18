#include "shift/Serialisation/sloader.h"
#include "shift/Properties/scontainer.h"
#include "shift/Properties/scontaineriterators.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/spropertytraits.h"

namespace Shift
{

void Loader::loadChildren(Container *parent)
  {
  if(beginChildren())
    {
    while(hasNextChild())
      {
      beginNextChild();
      read(parent);
      endNextChild();
      }
    endChildren();
    }
  }

void Loader::read(Container *read)
  {
  const PropertyInformation *info = type();
  xAssert(info);

  xAssert(info->functions().load);

  bool shouldLoad = true;
  shouldLoad = childHasValue();

  if(shouldLoad)
    {
    info->functions().load(read, *this);
    }
  else
    {
    // child has no stored value, because it was default at export, or something
    // so just load the property, with the Property method, and leave it as default.
    detail::PropertyBaseTraits::load(read, *this);
    }
  }

void Saver::saveChildren(const Container *c)
  {
  bool shouldSaveAnyChildren = false;

  xForeach(auto child, c->walker())
    {
    const PropertyInformation *info = child->typeInformation();

    if(info->functions().shouldSave(child))
      {
      shouldSaveAnyChildren = true;
      break;
      }
    }

  if(shouldSaveAnyChildren)
    {
    beginChildren();
    xForeach(auto child, c->walker())
      {
      const PropertyInformation *info = child->typeInformation();

      if(info->functions().shouldSave(child))
        {
        beginNextChild();
        //if(info->shouldSaveValue())
          {
          write(child);
          }
        endNextChild();
        }
      }
    endChildren();
    }
  }

void Saver::write(const Attribute *prop)
  {
  const PropertyInformation *info = prop->typeInformation();
  xAssert(info);
  xAssert(info->functions().save);

  info->functions().save(prop, *this);
  }

}
