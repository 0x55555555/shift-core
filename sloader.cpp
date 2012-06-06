#include "sloader.h"
#include "spropertycontainer.h"
#include "spropertyinformation.h"

void SLoader::loadChildren(SPropertyContainer *parent)
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

void SLoader::read(SPropertyContainer *read)
  {
  const SPropertyInformation *info = type();
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
    // so just load the property, with the SProperty method, and leave it as default.
    SProperty::loadProperty(read, *this);
    }
  }

void SSaver::saveChildren(const SPropertyContainer *c)
  {
  bool shouldSaveAnyChildren = false;

  for(const SProperty *child=c->firstChild(); child; child=child->nextSibling())
    {
    const SPropertyInformation *info = child->typeInformation();

    if(info->functions().shouldSave(child))
      {
      shouldSaveAnyChildren = true;
      break;
      }
    }

  if(shouldSaveAnyChildren)
    {
    beginChildren();
    for(const SProperty *child=c->firstChild(); child; child=child->nextSibling())
      {
      const SPropertyInformation *info = child->typeInformation();

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

void SSaver::write(const SProperty *prop)
  {
  const SPropertyInformation *info = prop->typeInformation();
  xAssert(info);
  xAssert(info->functions().save);

  info->functions().save(prop, *this);
  }
