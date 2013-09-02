#include "shift/TypeInformation/spropertytraits.h"
#include "shift/TypeInformation/spropertyinstanceinformation.h"
#include "shift/Properties/scontainer.h"
#include "shift/Properties/scontaineriterators.h"
#include "shift/Serialisation/sloader.h"
#include "XEventLogger"

namespace Shift
{

namespace detail
{

void PropertyBaseTraits::assign(const Attribute *, Attribute *)
  {
  }

void PropertyBaseTraits::save(const Attribute *p, Saver &l)
  {
  save(p, l, true);
  }

void PropertyBaseTraits::save(const Attribute *p, Saver &l, bool writeInput)
  {
  SProfileFunction
  const PropertyInformation *type = p->typeInformation();

  l.setType(type);

  l.beginAttribute("name");
  writeValue(l, p->name());
  l.endAttribute("name");

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

  if(const Property* prop = p->castTo<Property>())
    {
    if(writeInput && prop->input())
      {
      l.beginAttribute("input");
      writeValue(l, prop->input()->path(p));
      l.endAttribute("input");
      }
    }
  }

Attribute *PropertyBaseTraits::load(Container *parent, Loader &l)
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
  Name name;
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

  Attribute *attr = 0;
  if(dynamic != 0)
    {
    attr = parent->addAttribute(type, X_UINT8_SENTINEL, name, &initialiser);
    xAssert(attr);
    }
  else
    {
    attr = parent->findChild(name);
    xAssert(attr);
    xAssert(attr->typeInformation() == type);
    }

  if(Property *prop = attr->castTo<Property>())
    {
    l.beginAttribute("input");
    Loader::InputString input;
    readValue(l, input);
    l.endAttribute("input");

    if(!input.isEmpty())
      {
      l.resolveInputAfterLoad(prop, input);
      }
    }

  return attr;
  }

bool PropertyBaseTraits::shouldSaveValue(const Attribute *p)
  {
  if(const Property *prop = p->castTo<Property>())
    {
    if(prop->hasInput())
      {
      return false;
      }

    if(prop->isComputed())
      {
      return false;
      }
    }

  return true;
  }

bool PropertyBaseTraits::shouldSave(const Attribute *p)
  {
  if(p->isDynamic())
    {
    return true;
    }

  if(const Property *prop = p->castTo<Property>())
    {
    if(prop->hasInput())
      {
      xsize inputLocation = prop->embeddedBaseInstanceInformation()->defaultInput();
      if(inputLocation != 0)
        {
        const xuint8 *inputPropertyData = (xuint8*)p + inputLocation;

        const Property *inputProperty = (Property*)inputPropertyData;
        if(inputProperty != prop->input())
          {
          return true;
          }
        }
      else
        {
        return true;
        }
      }
    }

  const PropertyInformation *info = p->typeInformation();
  if(info->functions().shouldSaveValue(p))
    {
    return true;
    }

  return false;
  }


void PropertyContainerTraits::assign(const Attribute *f, Attribute *t)
  {
  SProfileFunction
  const Container *from = f->uncheckedCastTo<Container>();
  Container *to = t->uncheckedCastTo<Container>();

  if(from->containedProperties() == to->containedProperties())
    {
    xuint8 index = 0;
    auto tChildIt = to->walker().begin();
    xForeach(auto fChild, from->walker())
      {
      auto tChild = *tChildIt;

      if(!tChild || tChild->staticTypeInformation() != fChild->staticTypeInformation())
        {
        xAssert(tChild->isDynamic());
        if(tChild)
          {
          to->removeAttribute(tChild);
          }

        tChild = to->addAttribute(fChild->staticTypeInformation(), index);
        }

      tChild->assign(fChild);

      ++tChildIt;
      ++index;
      }
    }
  }

void PropertyContainerTraits::save(const Attribute *p, Saver &l)
  {
  SProfileFunction
  const Container *c = p->uncheckedCastTo<Container>();
  xAssert(c);

  detail::PropertyBaseTraits::save(p, l);

  l.saveChildren(c);
  }

Attribute *PropertyContainerTraits::load(Container *parent, Loader &l)
  {
  SProfileFunction
  xAssert(parent);

  Attribute *prop = detail::PropertyBaseTraits::load(parent, l);
  xAssert(prop);

  Container* container = prop->uncheckedCastTo<Container>();

  l.loadChildren(container);

  return prop;
  }

bool PropertyContainerTraits::shouldSaveValue(const Attribute *p)
  {
  const Container *ptr = p->uncheckedCastTo<Container>();
  if(ptr->containedProperties() < ptr->size())
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
