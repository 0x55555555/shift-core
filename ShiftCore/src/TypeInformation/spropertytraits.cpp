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

  bool dyn(p->isDynamic());
  if(dyn)
    {

    const PropertyInstanceInformation *instInfo = p->baseInstanceInformation();

    if(!instInfo->isDefaultMode())
      {
      const Eks::String &mode = instInfo->modeString();

      l.beginAttribute("mode");
      writeValue(l, mode);
      l.endAttribute("mode");
      }
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

  Initialiser initialiser;

  const auto data = l.currentData();

  l.beginAttribute("mode");
  readValue(l, initialiser.mode);
  l.endAttribute("mode");

  Attribute *attr = 0;
  if(data->dynamic != 0)
    {
    const PropertyInformation *type = data->type;
    xAssert(type);

    attr = parent->addAttribute(type, X_UINT8_SENTINEL, data->name, &initialiser);
    xAssert(attr);
    }
  else
    {
    attr = data->existing;
    xAssert(attr);
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
