#include "shift/TypeInformation/spropertytraits.h"
#include "shift/TypeInformation/spropertyinstanceinformation.h"
#include "shift/Properties/spropertycontainer.h"
#include "shift/Properties/spropertycontaineriterators.h"
#include "shift/Serialisation/sloader.h"

namespace Shift
{

namespace detail
{

void PropertyBaseTraits::assignProperty(const Property *, Property *)
  {
  }

void PropertyBaseTraits::saveProperty(const Property *p, Saver &l)
  {
  saveProperty(p, l, true);
  }

void PropertyBaseTraits::saveProperty(const Property *p, Saver &l, bool writeInput)
  {
  SProfileFunction
  const PropertyInformation *type = p->typeInformation();

  l.setType(type);

  l.beginAttribute("name");
  writeValue(l, p->name());
  l.endAttribute("name");

  xuint32 v(type->version());
  if(v != 0)
    {
    l.beginAttribute("version");
    writeValue(l, v);
    l.endAttribute("version");
    }

  bool dyn(p->isDynamic());
  if(dyn)
    {
    l.beginAttribute("dynamic");
    writeValue(l, dyn ? 1 : 0);
    l.endAttribute("dynamic");

    const PropertyInstanceInformation *instInfo = p->baseInstanceInformation();

    if(!instInfo->isDefaultMode())
      {
      const Eks::String &mode = instInfo->modeString();

      l.beginAttribute("mode");
      writeValue(l, mode);
      l.endAttribute("mode");
      }

#if 0
    xsize *affects = instInfo->affects();
    if(affects)
      {
      xAssert(p->parent());
      QString affectsString;

      const PropertyInformation *contInfo = p->parent()->typeInformation();
      while(*affects != 0)
        {
        const PropertyInstanceInformation *affectsInst = contInfo->child(*affects);

        xAssert(affectsInst);
        xAssert(!affectsInst->dynamic());
        if(!affectsInst->dynamic())
          {
          QString fixedName = affectsInst->name();
          affectsString.append(fixedName.replace(',', "\\,"));

          if(affects[1] != 0)
            {
            affectsString.append(",");
            }
          }

        ++affects;
        }

      l.beginAttribute("affects");
      writeValue(l, affectsString);
      l.endAttribute("affects");
      }
#endif
    }

  if(writeInput && p->input())
    {
    l.beginAttribute("input");
    writeValue(l, p->input()->path(p));
    l.endAttribute("input");
    }
  }

Property *PropertyBaseTraits::loadProperty(PropertyContainer *parent, Loader &l)
  {
  class Initialiser : public PropertyInstanceInformationInitialiser
    {
  public:
    //Initialiser() : affects(0) { }
    void initialise(PropertyInstanceInformation *inst)
      {
      //inst->setAffects(affects);
      inst->setModeString(mode);
      }

    //xsize *affects;
    QString mode;
    };

  SProfileFunction
  const PropertyInformation *type = l.type();
  xAssert(type);

  Initialiser initialiser;

  l.beginAttribute("name");
  PropertyName name;
  readValue(l, name);
  l.endAttribute("name");

  l.beginAttribute("dynamic");
  xuint32 dynamic=false;
  readValue(l, dynamic);
  l.endAttribute("dynamic");

  l.beginAttribute("mode");
  readValue(l, initialiser.mode);
  l.endAttribute("mode");

#if 0
  l.beginAttribute("affects");
  QString affectsString;
  readValue(l, affectsString);
  l.endAttribute("affects");
  if(!affectsString.isEmpty())
    {
    xsize numAffects = 0;
    bool escapeNext = false;
    for(int i=0, s=affectsString.size(); i<s; ++i)
      {
      xAssert(!escapeNext || affectsString[i] == ',');
      if((affectsString[i] == ',' || i == (s-1)) && !escapeNext)
        {
        numAffects++;
        }

      if(affectsString[i] == '\'')
        {
        escapeNext = true;
        }
      else
        {
        escapeNext = false;
        }
      }

###
    initialiser.affects = new xsize[numAffects+1];
    initialiser.affects[numAffects] = 0;
    const PropertyInformation *parentType = parent->typeInformation();

    int num = 0;
    int lastPos = 0;
    xsize affectsCount = 0;
    escapeNext = false;
    for(int i=0, s=affectsString.size(); i<s; ++i)
      {
      xAssert(!escapeNext || affectsString[i] == ',');
      if((affectsString[i] == ',' || i == (s-1)) && !escapeNext)
        {
        if(i == (s-1))
          {
          num++;
          }

        QString affectsName = affectsString.mid(lastPos, num);

        const PropertyInstanceInformation *inst = parentType->childFromName(affectsName);
        xAssert(inst);

        initialiser.affects[affectsCount++] = inst->location();

        num = 0;
        lastPos = i+1;
        }

      if(affectsString[i] == '\'')
        {
        escapeNext = true;
        }
      else
        {
        escapeNext = false;
        }
      num++;
      }

    xAssert(affectsCount == numAffects);
    }
#endif

  l.beginAttribute("version");
  xuint32 version=0;
  readValue(l, version);
  l.endAttribute("version");

  Property *prop = 0;
  if(dynamic != 0)
    {
    prop = parent->addProperty(type, X_SIZE_SENTINEL, name, &initialiser);
    xAssert(prop);
    }
  else
    {
    prop = parent->findChild(name);
    xAssert(prop);
    xAssert(prop->typeInformation() == type);
    }

  l.beginAttribute("input");
  Loader::InputString input;
  readValue(l, input);
  l.endAttribute("input");

  if(!input.isEmpty())
    {
    l.resolveInputAfterLoad(prop, input);
    }

  return prop;
  }

bool PropertyBaseTraits::shouldSavePropertyValue(const Property *p)
  {
  if(p->hasInput())
    {
    return false;
    }

  if(p->isComputed())
    {
    return false;
    }

  return true;
  }

bool PropertyBaseTraits::shouldSaveProperty(const Property *p)
  {
  if(p->isDynamic())
    {
    return true;
    }

  if(p->hasInput())
    {
    xsize inputLocation = p->embeddedBaseInstanceInformation()->defaultInput();
    if(inputLocation != 0)
      {
      const xuint8 *inputPropertyData = (xuint8*)p + inputLocation;

      const Property *inputProperty = (Property*)inputPropertyData;
      if(inputProperty != p->input())
        {
        return true;
        }
      }
    else
      {
      return true;
      }
    }

  const PropertyInformation *info = p->typeInformation();
  if(info->functions().shouldSaveValue(p))
    {
    return true;
    }

  return false;
  }


void PropertyContainerTraits::assignProperty(const Property *f, Property *t)
  {
  SProfileFunction
  const PropertyContainer *from = f->uncheckedCastTo<PropertyContainer>();
  PropertyContainer *to = t->uncheckedCastTo<PropertyContainer>();

  if(from->containedProperties() == to->containedProperties())
    {
    xsize index = 0;
    auto tChildIt = to->walker().begin();
    xForeach(auto fChild, from->walker())
      {
      auto tChild = *tChildIt;

      if(!tChild || tChild->staticTypeInformation() != fChild->staticTypeInformation())
        {
        xAssert(tChild->isDynamic());
        if(tChild)
          {
          to->removeProperty(tChild);
          }

        tChild = to->addProperty(fChild->staticTypeInformation(), index);
        }

      tChild->assign(fChild);

      ++tChildIt;
      ++index;
      }
    }
  else
    {
    xAssertFail();
    }
  }

void PropertyContainerTraits::saveProperty(const Property *p, Saver &l)
  {
  SProfileFunction
  const PropertyContainer *c = p->uncheckedCastTo<PropertyContainer>();
  xAssert(c);

  detail::PropertyBaseTraits::saveProperty(p, l);

  l.saveChildren(c);
  }

Property *PropertyContainerTraits::loadProperty(PropertyContainer *parent, Loader &l)
  {
  SProfileFunction
  xAssert(parent);

  Property *prop = detail::PropertyBaseTraits::loadProperty(parent, l);
  xAssert(prop);

  PropertyContainer* container = prop->uncheckedCastTo<PropertyContainer>();

  l.loadChildren(container);

  return prop;
  }

bool PropertyContainerTraits::shouldSavePropertyValue(const Property *p)
  {
  const PropertyContainer *ptr = p->uncheckedCastTo<PropertyContainer>();
  if(ptr->_containedProperties < ptr->size())
    {
    return true;
    }

  xForeach(auto p, ptr->walker())
    {
    const PropertyInformation *info = p->typeInformation();
    if(info->functions().shouldSave(p))
      {
      return true;
      }
    }

  return false;
  }

}

}
