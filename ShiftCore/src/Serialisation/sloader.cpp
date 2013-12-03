#include "shift/Serialisation/sloader.h"
#include "shift/Properties/scontainer.h"
#include "shift/Properties/scontainer.inl"
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
  bool shouldLoad = true;
  shouldLoad = childHasValue();


  beginAttribute("name");
  readValue(*this, _data.name);
  endAttribute("name");

  beginAttribute("dyn");
  int dynamic = 0;
  readValue(*this, dynamic);
  _data.dynamic = dynamic;    
  endAttribute("dyn");

  if(_data.dynamic)
    {
    beginAttribute("type");
    Name typeName;
    readValue(*this, typeName);
    endAttribute("type");
    _data.type = TypeRegistry::findType(typeName);
    }
  else
    {
    _data.existing = read->findChild(_data.name);
    xAssert(_data.existing);

    _data.type = _data.existing->typeInformation();
    }

  xAssert(_data.type);
  xAssert(_data.type->functions().load);

  if(shouldLoad)
    {
    _data.type->functions().load(read, *this);
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

  beginAttribute("name");
  writeValue(*this, prop->name());
  endAttribute("name");

  bool dyn(prop->isDynamic());
  if(dyn)
    {
    beginAttribute("type");
    writeValue(*this, info->typeName());
    endAttribute("type");

    beginAttribute("dyn");
    writeValue(*this, dyn ? 1 : 0);
    endAttribute("dyn");
    }

  xAssert(info);
  xAssert(info->functions().save);

  info->functions().save(prop, *this);
  }

}
