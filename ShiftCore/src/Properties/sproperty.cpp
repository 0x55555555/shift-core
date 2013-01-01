#include "shift/Properties/sproperty.h"
#include "shift/Properties/spropertycontaineriterators.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/sinterface.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/Changes/schange.h"
#include "shift/Changes/shandler.inl"
#include "shift/Utilities/sprocessmanager.h"
#include "shift/sentity.h"
#include "shift/sdatabase.h"
#include "QString"
#include "XProfiler"
#include "XConvertScriptSTL.h"

namespace Shift
{

static PropertyGroup::Information _sPropertyTypeInformation =
  Shift::propertyGroup().registerPropertyInformation(&_sPropertyTypeInformation,
                                                     Property::bootstrapStaticTypeInformation);

const PropertyInformation *Property::staticTypeInformation()
  {
  return _sPropertyTypeInformation.information;
  }

const PropertyInformation *Property::bootstrapStaticTypeInformation()
  {
  PropertyInformationTyped<Property>::bootstrapTypeInformation(
        &_sPropertyTypeInformation.information, "Property", 0);

  return staticTypeInformation();
  }

void Property::createTypeInformation(PropertyInformationTyped<Property> *info,
                                      const PropertyInformationCreateData &data)
  {
  if(data.registerInterfaces)
    {
    auto *api = info->apiInterface();

    typedef XScript::XMethodToGetter<Property, PropertyContainer * (), &Property::parent> ParentGetter;
    typedef XScript::XMethodToSetter<Property, PropertyContainer *, &Property::setParent> ParentSetter;

    typedef XScript::XMethodToGetter<Property, QVector<Property*> (), &Property::affects> AffectsGetter;

    static XScript::ClassDef<0,11,4> cls = {
      {
        api->property<const PropertyInformation *, &Property::typeInformation>("typeInformation"),
        api->property<Property *, const Property *, &Property::input, &Property::setInput>("input"),

        api->property<ParentGetter, ParentSetter>("parent"),

        api->property<Property *, &Property::output>("firstOutput"),
        api->property<Property *, &Property::nextOutput>("nextOutput"),

        api->property<Eks::String, &Property::mode>("mode"),
        api->property<bool, &Property::isDynamic>("dynamic"),
        api->property<const PropertyName &, const PropertyNameArg &, &Property::name, &Property::setName>("name"),

        api->property<QVariant, const QVariant &, &Property::value, &Property::setValue>("value"),
        api->property<Eks::String, &Property::valueAsString>("valueString"),


        api->property<AffectsGetter>("affects")
      },
      {
        api->constMethod<Eks::String (const Property *), &Property::pathTo>("pathTo"),

        api->method<void(), &Property::beginBlock>("beginBlock"),
        api->method<void(bool), &Property::endBlock>("endBlock"),

        api->constMethod<bool(const Property *), &Property::equals>("equals"),
      }
    };

    api->buildInterface(cls);

    info->addStaticInterface(new SBasicColourInterface);
    }
  }

void Property::setDependantsDirty()
  {
  for(Property *o=output(); o; o = o->nextOutput())
    {
    xAssert(o != o->nextOutput());
    o->setDirty();
    }

  const PropertyInstanceInformation *childBase = baseInstanceInformation();
  if(!isDynamic())
    {
    const EmbeddedPropertyInstanceInformation *child = childBase->embeddedInfo();
    const xsize *affectsLocations = child->affects();
    if(affectsLocations)
      {
      xuint8* parentLocation = (xuint8*)this;
      parentLocation -= child->location();

      for(;*affectsLocations; ++affectsLocations)
        {
        xuint8* affectedLocation = parentLocation + *affectsLocations;
        Property *affectsProp = (Property *)affectedLocation;

        xAssert(affectsProp);
        affectsProp->setDirty();
        }
      }
    }

  // ?  || isComputed()
  if(input() || _flags.hasFlag(Property::ParentHasInput))
    {
    PropertyContainer *c = castTo<PropertyContainer>();
    if(c)
      {
      xForeach(auto child, c->walker())
        {
        child->setDirty();
        }
      }
    }

  // if we know the parent has an output
  if(_flags.hasFlag(Property::ParentHasOutput))
    {
    Property *parent = this;

    if(_flags.hasFlag(Dirty))
      {
      while(parent->_flags.hasFlag(Property::ParentHasOutput)
            && !parent->_flags.hasFlag(PreGetting))
        {
        parent = parent->parent();

        parent->setDirty();
        }
      }

    while(parent->_flags.hasFlag(Property::ParentHasOutput))
      {
      parent = parent->parent();

      parent->setDependantsDirty();
      }
    }
  }

bool Property::NameChange::apply()
  {
  SProfileFunction
  property()->internalSetName(after(false));
  return true;
  }

bool Property::NameChange::unApply()
  {
  SProfileFunction
  property()->internalSetName(before(false));
  return true;
  }

bool Property::NameChange::inform(bool backwards)
  {
  SProfileFunction
  xAssert(property()->entity());
  property()->entity()->informTreeObservers(this, backwards );
  return true;
  }

Property::Property() : _input(0), _output(0), _nextOutput(0),
    _instanceInfo(0), _flags(Dirty)
#ifdef S_CENTRAL_CHANGE_HANDLER
    , _handler(0)
  #endif
#ifdef S_PROPERTY_USER_DATA
    , _userData(0)
#endif
  {
  }

Property::Property(const Property &) : _flags(Dirty)
  {
  xAssertFail();
  }

Property& Property::operator =(const Property &)
  {
  xAssertFail();
  return *this;
  }

#ifdef S_PROPERTY_USER_DATA
Property::~Property()
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

void Property::setName(const PropertyNameArg &in)
  {
  SProfileFunction
  xAssert(isDynamic());
  xAssert(parent());

  PropertyName fixedName;
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
  PropertyName realName = fixedName;
  while(parent()->findChild(realName))
    {
    realName = fixedName;
    realName.appendType(num++);
    }

  PropertyDoChange(NameChange, name(), realName, this);
  }

void Property::assignProperty(const Property *, Property *)
  {
  }

void Property::saveProperty(const Property *p, Saver &l)
  {
  saveProperty(p, l, true);
  }

void Property::saveProperty(const Property *p, Saver &l, bool writeInput)
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

Property *Property::loadProperty(PropertyContainer *parent, Loader &l)
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
  QString input;
  readValue(l, input);
  l.endAttribute("input");

  if(!input.isEmpty())
    {
    l.resolveInputAfterLoad(prop, input);
    }

  return prop;
  }

Handler *Property::handler()
  {
#ifdef S_CENTRAL_CHANGE_HANDLER
  return _handler;
#else
  return 0;
#endif
  }

const Handler *Property::handler() const
  {
  return const_cast<Property*>(this)->handler();
  }

Database *Property::database()
  {
#ifdef S_CENTRAL_CHANGE_HANDLER
  return handler()->database();
#else
  PropertyContainer *cont = parent();
  return cont->_database;
#endif
  }

const Database *Property::database() const
  {
#ifdef S_CENTRAL_CHANGE_HANDLER
  return handler()->database();
#else
  return parent()->_database;
#endif
  }

void Property::beginBlock()
  {
  handler()->beginBlock();
  }

void Property::endBlock(bool cancel)
  {
  handler()->endBlock(cancel);
  }

bool Property::equals(const Property *in) const
  {
  return this == in;
  }

bool Property::shouldSavePropertyValue(const Property *p)
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

bool Property::shouldSaveProperty(const Property *p)
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

const XScript::InterfaceBase *Property::apiInterface() const
  {
  return typeInformation()->apiInterface();
  }

const XScript::InterfaceBase *Property::staticApiInterface()
  {
  return staticTypeInformation()->apiInterface();
  }

bool Property::inheritsFromType(const PropertyInformation *type) const
  {
  SProfileFunction
  return typeInformation()->inheritsFromType(type);
  }

const PropertyName &Property::name() const
  {
  SProfileFunction
  return baseInstanceInformation()->name();
  }

PropertyName Property::escapedName() const
  {
  SProfileFunction
  const PropertyName &baseName =  baseInstanceInformation()->name();

  Eks::String::Replacement reps[] =
  {
    { Database::pathSeparator(), Database::escapedPathSeparator() }
  };

  Eks::String n;
  Eks::String::replace(baseName, &n, reps, X_ARRAY_COUNT(reps));

  return n;
  }

void Property::assign(const Property *propToAssign)
  {
  const PropertyInformation *info = typeInformation();
  xAssert(info);

  info->functions().assign(propToAssign, this);
  }

const Entity *Property::entity() const
  {
  return const_cast<Property*>(this)->entity();
  }

Entity *Property::entity()
  {
  SProfileFunction

  Property *prop = this;

  Entity *e = prop->castTo<Entity>();
  while(!e && prop)
    {
    prop = prop->parent();
    e = prop->castTo<Entity>();
    }

  return e;
  }

void Property::setParent(PropertyContainer *newParent)
  {
  parent()->moveProperty(newParent, this);
  }

PropertyContainer *Property::parent()
  {
  const PropertyInstanceInformation *inst = baseInstanceInformation();
  if(!inst->isDynamic())
    {
    return inst->embeddedInfo()->locateParent(this);
    }

  return inst->dynamicInfo()->parent();
  }

const PropertyContainer *Property::parent() const
  {
  return const_cast<Property*>(this)->parent();
  }

PropertyContainer *Property::embeddedParent()
  {
  const EmbeddedPropertyInstanceInformation *inst = embeddedBaseInstanceInformation();
  xAssert(inst);

  return inst->locateParent(this);
  }

const PropertyContainer *Property::embeddedParent() const
  {
  const EmbeddedPropertyInstanceInformation *inst = embeddedBaseInstanceInformation();
  xAssert(inst);

  return inst->locateConstParent(this);
  }

void Property::setInput(const Property *inp)
  {
  SProfileFunction
  if(inp)
    {
    PropertyDoChange(ConnectionChange, ConnectionChange::Connect, (Property*)inp, this);
    }
  else if(input())
    {
    PropertyDoChange(ConnectionChange, ConnectionChange::Disconnect, (Property*)input(), this);
    }
  }

void Property::connect(Property *prop) const
  {
  SProfileFunction
  if(prop && prop != this)
    {
    prop->setInput(this);
    }
  else
    {
    xAssertFail();
    }
  }

void Property::connect(const QVector<Property*> &l) const
  {
  if(l.size())
    {
    SBlock b(l.front()->handler());
    Q_FOREACH(Property *p, l)
      {
      connect(p);
      }
    }
  }

void Property::disconnect(Property *prop) const
  {
  xAssert(this == prop->input());
  prop->setInput(0);
  }

bool Property::isComputed() const
  {
  if(!isDynamic())
    {
    const EmbeddedPropertyInstanceInformation *staticInfo = embeddedBaseInstanceInformation();
    return staticInfo->compute() != 0;
    }
  return false;
  }

void Property::disconnect() const
  {
  SProfileFunction
  if(_input)
    {
    ((Property*)_input)->disconnect((Property*)this);
    }

  while(_output)
    {
    disconnect(_output);
    }
  }

QVector<const Property *> Property::affects() const
  {
  QVector<Property *> aff = const_cast<Property*>(this)->affects();
  return *reinterpret_cast<QVector<const Property*>*>(&aff);
  }

QVector<Property *> Property::affects()
  {
  QVector<Property *> ret;

  const EmbeddedPropertyInstanceInformation *info = embeddedBaseInstanceInformation();
  if(!info)
    {
    return ret;
    }

  xsize *affects = info->affects();
  if(!affects)
    {
    return ret;
    }

  PropertyContainer *par = parent();
  const PropertyInformation *parentInfo = par->typeInformation();
  while(*affects)
    {
    const EmbeddedPropertyInstanceInformation *affected = parentInfo->child(*affects);

    ret << affected->locateProperty(par);
    affects++;
    }

  return ret;
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
    _driver->setDependantsDirty();
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
    _driver->setDependantsDirty();
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

  PropertyContainer *cont = prop->castTo<PropertyContainer>();
  if(cont)
    {
    xForeach(auto child, cont->walker())
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
  PropertyContainer *cont = prop->castTo<PropertyContainer>();
  if(cont)
    {
    xForeach(auto child, cont->walker())
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
  PropertyContainer *cont = prop->castTo<PropertyContainer>();
  if(cont)
    {
    if(!cont->input() &&
        (cont->isDynamic() ||
         !cont->embeddedBaseInstanceInformation()->isComputed() ) &&
        !cont->_flags.hasFlag(Property::ParentHasInput))
      {
      xForeach(auto child, cont->walker())
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
  PropertyContainer *cont = prop->castTo<PropertyContainer>();
  if(cont)
    {
    if(!cont->output() &&
       (cont->isDynamic() ||
        !cont->embeddedBaseInstanceInformation()->affectsSiblings() ) &&
        !cont->_flags.hasFlag(Property::ParentHasOutput))
      {
      xForeach(auto child, cont->walker())
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

void Property::connectInternal(Property *prop) const
  {
  // prop can't already have an output
  if(prop->hasInput())
    {
    return;
    }
  prop->_input = (Property*)this;

  Property **output = (Property**)&_output;
  while(*output)
    {
    Property **nextOp = &((*output)->_nextOutput);
    output = nextOp;
    }

  if(output)
    {
    *output = prop;
    }

#ifdef X_DEBUG
  const Property *p = this;
  while(p)
    {
    xAssert(p != p->nextOutput());
    p = p->nextOutput();
    }
#endif
  }

void Property::disconnectInternal(Property *prop) const
  {
  xAssert(prop->_input == this);

  prop->_input = 0;

  Property **output = (Property**)&_output;
  while(*output)
    {
    if((*output) == prop)
      {
      (*output) = (*output)->_nextOutput;
      break;
      }
    else
      {
      output = &((*output)->_nextOutput);
      }
    }

  prop->_nextOutput = 0;

#ifdef X_DEBUG
  const Property *p = this;
  while(p)
    {
    xAssert(p != p->nextOutput());
    p = p->nextOutput();
    }
#endif
  }

Eks::String Property::pathTo(const Property *that) const
  {
  return that->path(this);
  }

Eks::String Property::path() const
  {
  SProfileFunction
  const Property *par = parent();
  if(par == 0)
    {
    return QString();
    }
  return par->path() + Database::pathSeparator() + escapedName();
  }

Eks::String Property::path(const Property *from) const
  {
  SProfileFunction

  if(from == this)
    {
    return "";
    }

  if(isDescendedFrom(from))
    {
    Eks::String ret;
    const Property *p = parent();
    while(p && p != from)
      {
      xAssert(p->name() != "");
      ret = p->escapedName() + Database::pathSeparator() + ret;

      p = p->parent();
      }
    return ret + escapedName();
    }

  const Property *parent = from->parent();
  if(parent)
    {
    Eks::String s("..");
    return s + Database::pathSeparator() + path(parent);
    }

  xAssert(0);
  return "";
  }

Eks::String Property::mode() const
  {
  return baseInstanceInformation()->modeString();
  }

bool Property::isDescendedFrom(const Property *in) const
  {
  SProfileFunction
  if(this == in)
    {
    return true;
    }

  const Property *par = parent();
  if(par == 0)
    {
    return false;
    }
  return par->isDescendedFrom(in);
  }

Property *Property::resolvePath(const Eks::String &path)
  {
  SProfileFunction
  preGet();

  Property *cur = this;

  Eks::String name;
  bool escape = false;
  for(int i = 0, s = path.size(); i < s; ++i)
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
          PropertyContainer* container = cur->castTo<PropertyContainer>();
          if(!container)
            {
            return 0;
            }

          xForeach(auto child, container->walker())
            {
            if(child->name() == name)
              {
              cur = child;
              break;
              }
            }
          }

        name.clear();
        }
      escape = false;
      }
    }
  return cur;
  }

const Property *Property::resolvePath(const Eks::String &path) const
  {
  return const_cast<Property*>(this)->resolvePath(path);
  }

QVariant Property::value() const
  {
  const PropertyVariantInterface *varInt = interface<PropertyVariantInterface>();

  if(varInt)
    {
    return varInt->asVariant(this);
    }
  return QString();
  }

void Property::setValue(const QVariant &val)
  {
  const PropertyVariantInterface *varInt = interface<PropertyVariantInterface>();

  if(varInt)
    {
    varInt->setVariant(this, val);
    }
  }

Eks::String Property::valueAsString() const
  {
  const PropertyVariantInterface *varInt = interface<PropertyVariantInterface>();

  if(varInt)
    {
    return varInt->asString(this);
    }
  return Eks::String();
  }

void Property::internalSetName(const PropertyNameArg &name)
  {
  name.toName(((BaseInstanceInformation*)this->baseInstanceInformation())->name());
  }

void Property::postSet()
  {
  SProfileFunction
  setDependantsDirty();

  _flags.clearFlag(Dirty);
  }

void Property::setDirty()
  {
  SProfileFunction
  if(!_flags.hasAnyFlags(Dirty|PreGetting))
  {
    _flags.setFlag(Dirty);
    setDependantsDirty();

#ifdef S_CENTRAL_CHANGE_HANDLER
    Entity *ent = entity();
    if(ent)
      {
      ent->informDirtyObservers(this);
      }
#endif
    }
  }

void Property::updateParent() const
  {
  const Property *par = parent();
  if(par)
    {
    par->preGet();
    }
  }

void Property::update() const
  {
  SProfileFunction
  Database *db = const_cast<Database*>(database());
  bool stateStorageEnabled = db->stateStorageEnabled();
  db->setStateStorageEnabled(false);

  Property *prop = const_cast<Property*>(this);
  prop->_flags.setFlag(PreGetting);

  // this is a const function, but because we delay computation we may need to assign here
  prop->_flags.clearFlag(Dirty);



  if(!isDynamic())
    {
    const EmbeddedInstanceInformation *child = embeddedBaseInstanceInformation();
    EmbeddedInstanceInformation::ComputeFunction compute = child->compute();
    if(compute)
      {
      PropertyContainer *par = prop->embeddedParent();
      xAssert(par);

      compute(child, par);
      }
    else if(input())
      {
      prop->assign(input());
      }
    }
  else if(input())
    {
    prop->assign(input());
    }

  // dirty can be set again in compute functions.
  prop->_flags.clearFlag(Dirty);
  prop->_flags.clearFlag(PreGetting);

  db->setStateStorageEnabled(stateStorageEnabled);

  xAssert(!_flags.hasFlag(Dirty));
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

InterfaceBase *Property::interface(xuint32 typeId)
  {
  const InterfaceBaseFactory* factory = typeInformation()->interfaceFactory(typeId);
  if(factory)
    {
    return const_cast<InterfaceBaseFactory*>(factory)->classInterface(this);
    }
  return 0;
  }

const InterfaceBase *Property::interface(xuint32 typeId) const
  {
  const InterfaceBaseFactory* factory = typeInformation()->interfaceFactory(typeId);
  if(factory)
    {
    return const_cast<InterfaceBaseFactory*>(factory)->classInterface(const_cast<Property*>(this));
    }
  return 0;

  }

}
