#include "shift/Properties/sattribute.h"
#include "shift/Properties/scontaineriterators.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/Changes/spropertychanges.h"
#include "shift/TypeInformation/sinterface.h"
#include "shift/sentity.h"
#include "shift/sdatabase.h"
#include "shift/Changes/shandler.h"
#include "shift/Changes/shandler.inl"
#include "Containers/XStringBuffer.h"

namespace Shift
{

static Module::Information _sAttribyteTypeInformation =
  Shift::shiftModule().registerPropertyInformation(_sAttribyteTypeInformation,
                                                     Attribute::bootstrapStaticTypeInformation);

Shift::Module &Attribute::module()
  {
  return Shift::shiftModule();
  }

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

void Attribute::createTypeInformation(
    PropertyInformationTyped<Attribute> *,
    const PropertyInformationCreateData &)
  {
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
  parent()->makeUniqueName(this, in, fixedName);

  if(name() == fixedName)
    {
    return;
    }

  forceSetName(fixedName);
  }

void Attribute::forceSetName(const Name &in)
  {
  PropertyDoChange(NameChange, name(), in, this);
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
  
  Eks::TemporaryAllocator alloc(temporaryAllocator());

  Eks::String n(&alloc);
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

Eks::String Attribute::pathTo(const Attribute *that, Eks::AllocatorBase* allocator) const
  {
  return that->path(this, allocator);
  }

Eks::String Attribute::path(Eks::AllocatorBase* allocator) const
  {
  SProfileFunction
  const Property *par = parent();
  if(par == 0)
    {
    return Eks::String();
    }
  return par->path(allocator) + Database::pathSeparator() + escapedIdentifier();
  }

Eks::String Attribute::path(const Attribute *from, Eks::AllocatorBase* allocator) const
  {
  SProfileFunction

  if(from == this)
    {
    return Eks::String("", allocator);
    }

  if(isDescendedFrom(from))
    {
    Eks::String ret(allocator);
    const Attribute *p = parent();
    while(p && p != from)
      {
      xAssert(p->identifier() != "");

      Eks::String tmp(allocator);
      tmp += p->escapedIdentifier();
      tmp += Database::pathSeparator();

      ret = tmp + ret;

      p = p->parent();
      }
    return ret + escapedIdentifier();
    }

  const Attribute *parent = from->parent();
  if(parent)
    {
    Eks::String s("..", allocator);
    s += Database::pathSeparator();
    s += path(parent, allocator);
    return s;
    }

  xAssert(0);
  return Eks::String("", allocator);
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

          cur = container->findIdentifier(name);
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
