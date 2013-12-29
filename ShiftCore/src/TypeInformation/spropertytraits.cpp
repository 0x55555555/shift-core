#include "shift/TypeInformation/spropertytraits.h"
#include "shift/TypeInformation/spropertyinstanceinformation.h"
#include "shift/Properties/scontainer.h"
#include "shift/Properties/scontainer.inl"
#include "shift/Properties/scontaineriterators.h"
#include "shift/Serialisation/sloader.h"
#include "shift/Serialisation/sattributeio.h"
#include "Utilities/XEventLogger.h"
#include "Memory/XTemporaryAllocator.h"

namespace Shift
{

namespace detail
{

void PropertyBaseTraits::assign(const Attribute *, Attribute *)
  {
  }

void PropertyBaseTraits::save(const Attribute *p, AttributeSaver &l)
  {
  save(p, l, true);
  }

void PropertyBaseTraits::save(const Attribute *p, AttributeSaver &l, bool writeInput)
  {
  SProfileFunction

  Eks::TemporaryAllocator alloc(p->temporaryAllocator());

  bool dyn(p->isDynamic());
  if(dyn)
    {
    const PropertyInstanceInformation *instInfo = p->baseInstanceInformation();

    if(!instInfo->isDefaultMode())
      {
      const Eks::String &mode = instInfo->modeString();

      l.write(l.modeSymbol(), mode);
      }
    }

  if(const Property* prop = p->castTo<Property>())
    {
    if(writeInput && prop->input())
      {
      Eks::String path = prop->input()->path(p, &alloc);

      l.write(l.inputSymbol(), path);
      }
    }
  }

Attribute *PropertyBaseTraits::load(Container *parent, AttributeLoader &l)
  {
  class Initialiser : public PropertyInstanceInformationInitialiser
    {
  public:
    void initialise(PropertyInstanceInformation *inst)
      {
      inst->setModeString(mode);
      }

    Eks::String mode;
    };

  SProfileFunction

  Initialiser initialiser;

  Eks::TemporaryAllocator alloc(parent->temporaryAllocator());

  initialiser.mode = Eks::String(&alloc);
  l.read(l.modeSymbol(), initialiser.mode);

  auto attr = l.existingAttribute();
  if(!attr)
    {
    const PropertyInformation *type = l.type();
    xAssert(type);

    attr = parent->addAttribute(type, X_SIZE_SENTINEL, l.name(), &initialiser);
    xAssert(attr);
    }


  if(Property *prop = attr->castTo<Property>())
    {
    Eks::String input = Eks::String(&alloc);

    if(l.read(l.inputSymbol(), input))
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
