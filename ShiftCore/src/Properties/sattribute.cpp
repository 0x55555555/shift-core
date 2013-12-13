#include "shift/Properties/sattribute.h"
#include "shift/Properties/scontaineriterators.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/Changes/spropertychanges.h"
#include "shift/TypeInformation/sinterface.h"
#include "shift/TypeInformation/sinterfaces.h"
#include "shift/sentity.h"
#include "shift/sdatabase.h"
#include "shift/Changes/shandler.h"
#include "shift/Changes/shandler.inl"
#include "Containers/XStringBuffer.h"

namespace Shift
{

static PropertyGroup::Information _sAttribyteTypeInformation =
  Shift::propertyGroup().registerPropertyInformation(&_sAttribyteTypeInformation,
                                                     Attribute::bootstrapStaticTypeInformation);

const PropertyInformation *Attribute::staticTypeInformation()
  {
  return _sAttribyteTypeInformation.information;
  }

const PropertyInformation *Attribute::bootstrapStaticTypeInformation(Eks::AllocatorBase *allocator)
  {
  PropertyInformationTyped<Attribute>::bootstrapTypeInformation(
        &_sAttribyteTypeInformation.information, "Attribute", 0, allocator);

  return staticTypeInformation();
  }

void Attribute::createTypeInformation(PropertyInformationTyped<Attribute> *info,
                                      const PropertyInformationCreateData &data)
  {
  if(data.registerInterfaces)
    {
    auto *api = info->apiInterface();

    typedef XScript::XMethodToGetter<Attribute, Container * (), &Property::parent> ParentGetter;
    typedef XScript::XMethodToSetter<Attribute, Container *, &Property::setParent> ParentSetter;

    static XScript::ClassDef<0,6,4> cls = {
      {
        api->property<const PropertyInformation *, &Property::typeInformation>("typeInformation"),

        //api->property<ParentGetter, ParentSetter>("parent"),

        api->property<const Eks::String &, &Property::mode>("mode"),
        api->property<bool, &Property::isDynamic>("dynamic"),
        api->property<const Name &, const NameArg &, &Property::name, &Property::setName>("name"),

        api->property<QVariant, const QVariant &, &Property::value, &Property::setValue>("value"),
        api->property<Eks::String, &Property::valueAsString>("valueString"),

      },
      {
        api->constMethod<Eks::String (const Attribute *), &Attribute::pathTo>("pathTo"),

        api->method<void(), &Attribute::beginBlock>("beginBlock"),
        api->method<void(bool), &Attribute::endBlock>("endBlock"),

        api->constMethod<bool(const Attribute *), &Attribute::equals>("equals"),
      }
    };

    api->buildInterface(cls);
    }
  }

Attribute::Attribute() : _instanceInfo(0)
#ifdef S_PROPERTY_USER_DATA
    , _userData(0)
#endif
  {
  }

Attribute::Attribute(const Attribute &)
  {
  xAssertFail();
  }

Attribute& Attribute::operator =(const Attribute &)
  {
  xAssertFail();
  return *this;
  }

#ifdef S_PROPERTY_USER_DATA
Attribute::~Attribute()
  {
  UserData *ud = _userData;
  while(ud)
    {
    UserData *next = ud->_next;
    if(ud->onPropertyDelete(this))
      {
      delete ud;
      }
    ud = next;
    }
  }
#endif

void Attribute::setName(const NameArg &in)
  {
  SProfileFunction
  xAssert(isDynamic());
  xAssert(parent());
  xAssert(parent()->hasNamedChildren());

  Name fixedName;
  in.toName(fixedName);
  if(fixedName == "")
    {
    fixedName = typeInformation()->typeName();
    }

  if(name() == fixedName)
    {
    return;
    }

  // ensure the name is unique
  xsize num = 1;
  Name realName = fixedName;
  while(parent()->findChild(realName))
    {
    realName = fixedName;
    realName.appendType(num++);
    }

  PropertyDoChange(NameChange, name(), realName, this);
  }

Handler *Attribute::handler()
  {
#ifdef S_CENTRAL_CHANGE_HANDLER
  return entity()->handler();
#else
  return 0;
#endif
  }

const Handler *Attribute::handler() const
  {
  return const_cast<Attribute*>(this)->handler();
  }

Database *Attribute::database()
  {
#ifdef S_CENTRAL_CHANGE_HANDLER
  return handler()->database();
#else
  Entity *cont = entity();
  return cont->_database;
#endif
  }

const Database *Attribute::database() const
  {
#ifdef S_CENTRAL_CHANGE_HANDLER
  return handler()->database();
#else
  return entity()->_database;
#endif
  }

void Attribute::beginBlock()
  {
  handler()->beginBlock();
  }

void Attribute::endBlock(bool cancel)
  {
  handler()->endBlock(cancel);
  }

bool Attribute::equals(const Attribute *in) const
  {
  return this == in;
  }

const XScript::InterfaceBase *Attribute::apiInterface() const
  {
  return typeInformation()->apiInterface();
  }

const XScript::InterfaceBase *Attribute::staticApiInterface()
  {
  return staticTypeInformation()->apiInterface();
  }

Eks::TemporaryAllocatorCore *Attribute::temporaryAllocator() const
  {
#ifdef S_CENTRAL_CHANGE_HANDLER
  return handler()->temporaryAllocator();
#else
  return TypeRegistry::temporaryAllocator();
#endif
  }

Eks::AllocatorBase *Attribute::persistentBlockAllocator() const
  {
#ifdef S_CENTRAL_CHANGE_HANDLER
  return handler()->persistentBlockAllocator();
#else
  return TypeRegistry::persistentBlockAllocator();
#endif
  }

Eks::AllocatorBase *Attribute::generalPurposeAllocator() const
  {
#ifdef S_CENTRAL_CHANGE_HANDLER
  return handler()->generalPurposeAllocator();
#else
  return TypeRegistry::generalPurposeAllocator();
#endif
  }


bool Attribute::inheritsFromType(const PropertyInformation *type) const
  {
  SProfileFunction
  return typeInformation()->inheritsFromType(type);
  }

const Name &Attribute::name() const
  {
  SProfileFunction
  return baseInstanceInformation()->name();
  }

Name Attribute::identifier() const
  {
  SProfileFunction
  if (parent()->hasNamedChildren())
    {
    return baseInstanceInformation()->name();
    }
  else
    {
    Name out;
    Name::Buffer buf(&out);
    Eks::String::OStream stream(&buf);

    stream << parent()->index(this);

    return out;
    }
  }

Name Attribute::escapedIdentifier() const
  {
  SProfileFunction
  Name baseName = identifier();

  Eks::String::Replacement reps[] =
  {
    { Database::pathSeparator(), Database::escapedPathSeparator() }
  };

  Eks::String n;
  Eks::String::replace(baseName, &n, reps, X_ARRAY_COUNT(reps));

  return n;
  }

void Attribute::assign(const Attribute *propToAssign)
  {
  const PropertyInformation *info = typeInformation();
  xAssert(info);

  info->functions().assign(propToAssign, this);
  }

const Entity *Attribute::entity() const
  {
  return const_cast<Attribute*>(this)->entity();
  }

Entity *Attribute::entity()
  {
  SProfileFunction

  Attribute *prop = this;

  Entity *e = prop->castTo<Entity>();
  while(!e && prop)
    {
    prop = prop->parent();
    xAssert(prop);
    e = prop->castTo<Entity>();
    }

  return e;
  }

void Attribute::setParent(Container *newParent)
  {
  parent()->moveAttribute(newParent, this);
  }

Container *Attribute::parent()
  {
  const PropertyInstanceInformation *inst = baseInstanceInformation();
  if(!inst->isDynamic())
    {
    return inst->embeddedInfo()->locateParent(this);
    }

  return inst->dynamicInfo()->parent();
  }

const Container *Attribute::parent() const
  {
  return const_cast<Attribute*>(this)->parent();
  }

Container *Attribute::embeddedParent()
  {
  const EmbeddedPropertyInstanceInformation *inst = embeddedBaseInstanceInformation();
  xAssert(inst);

  return inst->locateParent(this);
  }

const Container *Attribute::embeddedParent() const
  {
  const EmbeddedPropertyInstanceInformation *inst = embeddedBaseInstanceInformation();
  xAssert(inst);

  return inst->locateConstParent(this);
  }

Eks::String Attribute::pathTo(const Attribute *that) const
  {
  return that->path(this);
  }

Eks::String Attribute::path() const
  {
  SProfileFunction
  const Property *par = parent();
  if(par == 0)
    {
    return Eks::String();
    }
  return par->path() + Database::pathSeparator() + escapedIdentifier();
  }

Eks::String Attribute::path(const Attribute *from) const
  {
  SProfileFunction

  if(from == this)
    {
    return "";
    }

  if(isDescendedFrom(from))
    {
    Eks::String ret;
    const Attribute *p = parent();
    while(p && p != from)
      {
      xAssert(p->identifier() != "");
      ret = p->escapedIdentifier() + Database::pathSeparator() + ret;

      p = p->parent();
      }
    return ret + escapedIdentifier();
    }

  const Attribute *parent = from->parent();
  if(parent)
    {
    Eks::String s("..");
    return s + Database::pathSeparator() + path(parent);
    }

  xAssert(0);
  return "";
  }

const Eks::String &Attribute::mode() const
  {
  return baseInstanceInformation()->modeString();
  }

bool Attribute::isDescendedFrom(const Attribute *in) const
  {
  SProfileFunction
  if(this == in)
    {
    return true;
    }

  const Attribute *par = parent();
  if(par == 0)
    {
    return false;
    }
  return par->isDescendedFrom(in);
  }

Attribute *Attribute::resolvePath(const Eks::String &path)
  {
  SProfileFunction

  Attribute *cur = this;

  Eks::String name;
  bool escape = false;
  for(xsize i = 0, s = path.size(); i < s; ++i)
    {
    Eks::String::Char c = path[i];

    if(c == Eks::String::Char('\\'))
      {
      escape = true;
      }
    else
      {
      if(!escape && c != Eks::String::Char('/'))
        {
        name.pushBack(c);
        }

      if(!escape && (c == Eks::String::Char('/') || i == (s-1)))
        {
        if(!cur)
          {
          return 0;
          }

        if(name == "..")
          {
          cur = cur->parent();
          }
        else
          {
          Container* container = cur->castTo<Container>();
          if(!container)
            {
            return 0;
            }

          cur = container->findChild(name);
          }

        name.clear();
        }
      escape = false;
      }
    }
  return cur;
  }

const Attribute *Attribute::resolvePath(const Eks::String &path) const
  {
  return const_cast<Attribute*>(this)->resolvePath(path);
  }

QVariant Attribute::value() const
  {
  const PropertyVariantInterface *varInt = findInterface<PropertyVariantInterface>();

  if(varInt)
    {
    return varInt->asVariant(this);
    }
  return QVariant();
  }

void Attribute::setValue(const QVariant &val)
  {
  const PropertyVariantInterface *varInt = findInterface<PropertyVariantInterface>();

  if(varInt)
    {
    varInt->setVariant(this, val);
    }
  }

Eks::String Attribute::valueAsString() const
  {
  const PropertyVariantInterface *varInt = findInterface<PropertyVariantInterface>();

  if(varInt)
    {
    return varInt->asString(this);
    }
  return Eks::String();
  }

void Attribute::internalSetName(const NameArg &name)
  {
  name.toName(((BaseInstanceInformation*)this->baseInstanceInformation())->name());
  }

#ifdef S_PROPERTY_USER_DATA
void Property::addUserData(UserData *userData)
  {
  xAssert(userData);
  xAssert(!userData->next());
  if(userData && !userData->next())
    {
    userData->_next = _userData;
    _userData = userData;
    }
  }

void Property::removeUserData(UserData *userData)
  {
  xAssert(userData);
  xAssert(userData->_next);
  UserData *last = 0;
  UserData *ud = _userData;
  while(ud)
    {
    if(ud == userData)
      {
      if(last)
        {
        last->_next = userData->_next;
        }
      else
        {
        _userData = userData->_next;
        }
      userData->_next = 0;
      break;
      }
    last = ud;
    ud = ud->next();
    }
  xAssert(!userData->_next);
  }
#endif

InterfaceBase *Attribute::findInterface(xuint32 typeId)
  {
  const InterfaceBaseFactory* factory = TypeRegistry::interfaceFactory(typeInformation(), typeId);
  if(factory)
    {
    return const_cast<InterfaceBaseFactory*>(factory)->classInterface(this);
    }
  return 0;
  }

const InterfaceBase *Attribute::findInterface(xuint32 typeId) const
  {
  const InterfaceBaseFactory* factory = TypeRegistry::interfaceFactory(typeInformation(), typeId);
  if(factory)
    {
    return const_cast<InterfaceBaseFactory*>(factory)->classInterface(const_cast<Attribute*>(this));
    }
  return 0;
  }


}
