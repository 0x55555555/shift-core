#include "sproperty.h"
#include "spropertyinformation.h"
#include "sinterface.h"
#include "sentity.h"
#include "sdatabase.h"
#include "schange.h"
#include "QString"
#include "sprocessmanager.h"
#include "XProfiler"
#include "styperegistry.h"
#include "spropertycontaineriterators.h"
#include "spropertyinformationhelpers.h"
#include "XConvertScriptSTL.h"
#include "shandlerimpl.h"

static SPropertyGroup::Information _sPropertyTypeInformation =
  Shift::propertyGroup().registerPropertyInformation(&_sPropertyTypeInformation,
                                                     SProperty::bootstrapStaticTypeInformation);

const SPropertyInformation *SProperty::staticTypeInformation()
  {
  return _sPropertyTypeInformation.information;
  }

const SPropertyInformation *SProperty::bootstrapStaticTypeInformation()
  {
  SPropertyInformationTyped<SProperty>::bootstrapTypeInformation(
        &_sPropertyTypeInformation.information, "SProperty", 0);

  return staticTypeInformation();
  }

void SProperty::createTypeInformation(SPropertyInformationTyped<SProperty> *info,
                                      const SPropertyInformationCreateData &data)
  {
  if(data.registerInterfaces)
    {
    auto *api = info->apiInterface();

    typedef XScript::XMethodToGetter<SProperty, SPropertyContainer * (), &SProperty::parent> ParentGetter;
    typedef XScript::XMethodToSetter<SProperty, SPropertyContainer *, &SProperty::setParent> ParentSetter;

    typedef XScript::XMethodToGetter<SProperty, QVector<SProperty*> (), &SProperty::affects> AffectsGetter;

    static XScript::ClassDef<0,11,4> cls = {
      {
        api->property<const SPropertyInformation *, &SProperty::typeInformation>("typeInformation"),
        api->property<SProperty *, const SProperty *, &SProperty::input, &SProperty::setInput>("input"),

        api->property<ParentGetter, ParentSetter>("parent"),

        api->property<SProperty *, &SProperty::output>("firstOutput"),
        api->property<SProperty *, &SProperty::nextOutput>("nextOutput"),

        api->property<QString, &SProperty::mode>("mode"),
        api->property<bool, &SProperty::isDynamic>("dynamic"),
        api->property<const QString &, const QString &, &SProperty::name, &SProperty::setName>("name"),

        api->property<QVariant, const QVariant &, &SProperty::value, &SProperty::setValue>("value"),
        api->property<QString, &SProperty::valueAsString>("valueString"),


        api->property<AffectsGetter>("affects")
      },
      {
        api->constMethod<QString (const SProperty *), &SProperty::pathTo>("pathTo"),

        api->method<void(), &SProperty::beginBlock>("beginBlock"),
        api->method<void(bool), &SProperty::endBlock>("endBlock"),

        api->constMethod<bool(const SProperty *), &SProperty::equals>("equals"),
      }
    };

    api->buildInterface(cls);

    info->addStaticInterface(new SBasicColourInterface);
    }
  }

void SProperty::setDependantsDirty()
  {
  for(SProperty *o=output(); o; o = o->nextOutput())
    {
    xAssert(o != o->nextOutput());
    o->setDirty();
    }

  const SPropertyInstanceInformation *childBase = baseInstanceInformation();
  if(!isDynamic())
    {
    const SEmbeddedPropertyInstanceInformation *child = childBase->embeddedInfo();
    const xsize *affectsLocations = child->affects();
    if(affectsLocations)
      {
      xuint8* parentLocation = (xuint8*)this;
      parentLocation -= child->location();

      for(;*affectsLocations; ++affectsLocations)
        {
        xuint8* affectedLocation = parentLocation + *affectsLocations;
        SProperty *affectsProp = (SProperty *)affectedLocation;

        xAssert(affectsProp);
        affectsProp->setDirty();
        }
      }
    }

  // ?  || isComputed()
  if(input() || _flags.hasFlag(SProperty::ParentHasInput))
    {
    SPropertyContainer *c = castTo<SPropertyContainer>();
    if(c)
      {
      xForeach(auto child, c->walker())
        {
        child->setDirty();
        }
      }
    }

  // if we know the parent has an output
  if(_flags.hasFlag(SProperty::ParentHasOutput))
    {
    SProperty *parent = this;

    if(_flags.hasFlag(Dirty))
      {
      while(parent->_flags.hasFlag(SProperty::ParentHasOutput)
            && !parent->_flags.hasFlag(PreGetting))
        {
        parent = parent->parent();

        parent->setDirty();
        }
      }

    while(parent->_flags.hasFlag(SProperty::ParentHasOutput))
      {
      parent = parent->parent();

      parent->setDependantsDirty();
      }
    }
  }

bool SProperty::NameChange::apply()
  {
  SProfileFunction
  property()->internalSetName(after(false));
  return true;
  }

bool SProperty::NameChange::unApply()
  {
  SProfileFunction
  property()->internalSetName(before(false));
  return true;
  }

bool SProperty::NameChange::inform(bool backwards)
  {
  SProfileFunction
  xAssert(property()->entity());
  property()->entity()->informTreeObservers(this, backwards );
  return true;
  }

SProperty::SProperty() : _input(0), _output(0), _nextOutput(0),
    _instanceInfo(0), _flags(Dirty)
#ifdef S_CENTRAL_CHANGE_HANDLER
    , _handler(0)
  #endif
#ifdef S_PROPERTY_USER_DATA
    , _userData(0)
#endif
  {
  }

SProperty::SProperty(const SProperty &) : _flags(Dirty)
  {
  xAssertFail();
  }

SProperty& SProperty::operator =(const SProperty &)
  {
  xAssertFail();
  return *this;
  }

#ifdef S_PROPERTY_USER_DATA
SProperty::~SProperty()
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

void SProperty::setName(const QString &in)
  {
  SProfileFunction
  xAssert(isDynamic());
  xAssert(parent());

  QString fixedName = in;
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
  QString realName = fixedName;
  while(parent()->findChild(realName))
    {
    realName = fixedName + QString::number(num++);
    }

  SPropertyDoChange(NameChange, name(), realName, this);
  }

void SProperty::assignProperty(const SProperty *, SProperty *)
  {
  }

void SProperty::saveProperty(const SProperty *p, SSaver &l)
  {
  saveProperty(p, l, true);
  }

void SProperty::saveProperty(const SProperty *p, SSaver &l, bool writeInput)
  {
  SProfileFunction
  const SPropertyInformation *type = p->typeInformation();

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

    const SPropertyInstanceInformation *instInfo = p->baseInstanceInformation();

    if(!instInfo->isDefaultMode())
      {
      QString mode = instInfo->modeString();

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

      const SPropertyInformation *contInfo = p->parent()->typeInformation();
      while(*affects != 0)
        {
        const SPropertyInstanceInformation *affectsInst = contInfo->child(*affects);

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

SProperty *SProperty::loadProperty(SPropertyContainer *parent, SLoader &l)
  {
  class Initialiser : public SPropertyInstanceInformationInitialiser
    {
  public:
    //Initialiser() : affects(0) { }
    void initialise(SPropertyInstanceInformation *inst)
      {
      //inst->setAffects(affects);
      inst->setModeString(mode);
      }

    //xsize *affects;
    QString mode;
    };

  SProfileFunction
  const SPropertyInformation *type = l.type();
  xAssert(type);

  Initialiser initialiser;

  l.beginAttribute("name");
  QString name;
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
    const SPropertyInformation *parentType = parent->typeInformation();

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

        const SPropertyInstanceInformation *inst = parentType->childFromName(affectsName);
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

  SProperty *prop = 0;
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

SHandler *SProperty::handler()
  {
#ifdef S_CENTRAL_CHANGE_HANDLER
  return _handler;
#else
  return 0;
#endif
  }

const SHandler *SProperty::handler() const
  {
  return const_cast<SProperty*>(this)->handler();
  }

SDatabase *SProperty::database()
  {
#ifdef S_CENTRAL_CHANGE_HANDLER
  return handler()->database();
#else
  SPropertyContainer *cont = parent();
  return cont->_database;
#endif
  }

const SDatabase *SProperty::database() const
  {
#ifdef S_CENTRAL_CHANGE_HANDLER
  return handler()->database();
#else
  return parent()->_database;
#endif
  }

void SProperty::beginBlock()
  {
  handler()->beginBlock();
  }

void SProperty::endBlock(bool cancel)
  {
  handler()->endBlock(cancel);
  }

bool SProperty::equals(const SProperty *in) const
  {
  return this == in;
  }

bool SProperty::shouldSavePropertyValue(const SProperty *p)
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

bool SProperty::shouldSaveProperty(const SProperty *p)
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

      const SProperty *inputProperty = (SProperty*)inputPropertyData;
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

  const SPropertyInformation *info = p->typeInformation();
  if(info->functions().shouldSaveValue(p))
    {
    return true;
    }

  return false;
  }

const XScript::InterfaceBase *SProperty::apiInterface() const
  {
  return typeInformation()->apiInterface();
  }

const XScript::InterfaceBase *SProperty::staticApiInterface()
  {
  return staticTypeInformation()->apiInterface();
  }

bool SProperty::inheritsFromType(const SPropertyInformation *type) const
  {
  SProfileFunction
  return typeInformation()->inheritsFromType(type);
  }

const QString &SProperty::name() const
  {
  SProfileFunction
  return baseInstanceInformation()->name();
  }

QString SProperty::escapedName() const
  {
  SProfileFunction
  QString n = baseInstanceInformation()->name();

  n.replace(SDatabase::pathSeparator(), '\\' + SDatabase::pathSeparator());

  return n;
  }

void SProperty::assign(const SProperty *propToAssign)
  {
  const SPropertyInformation *info = typeInformation();
  xAssert(info);

  info->functions().assign(propToAssign, this);
  }

const SEntity *SProperty::entity() const
  {
  return const_cast<SProperty*>(this)->entity();
  }

SEntity *SProperty::entity()
  {
  SProfileFunction

  SProperty *prop = this;

  SEntity *e = prop->castTo<SEntity>();
  while(!e && prop)
    {
    prop = prop->parent();
    e = prop->castTo<SEntity>();
    }

  return e;
  }

void SProperty::setParent(SPropertyContainer *newParent)
  {
  parent()->moveProperty(newParent, this);
  }

SPropertyContainer *SProperty::parent()
  {
  const SPropertyInstanceInformation *inst = baseInstanceInformation();
  if(!inst->isDynamic())
    {
    return inst->embeddedInfo()->locateParent(this);
    }

  return inst->dynamicInfo()->parent();
  }

const SPropertyContainer *SProperty::parent() const
  {
  return const_cast<SProperty*>(this)->parent();
  }

SPropertyContainer *SProperty::embeddedParent()
  {
  const SEmbeddedPropertyInstanceInformation *inst = embeddedBaseInstanceInformation();
  xAssert(inst);

  return inst->locateParent(this);
  }

const SPropertyContainer *SProperty::embeddedParent() const
  {
  const SEmbeddedPropertyInstanceInformation *inst = embeddedBaseInstanceInformation();
  xAssert(inst);

  return inst->locateConstParent(this);
  }

void SProperty::setInput(const SProperty *inp)
  {
  SProfileFunction
  if(inp)
    {
    SPropertyDoChange(ConnectionChange, ConnectionChange::Connect, (SProperty*)inp, this);
    }
  else if(input())
    {
    SPropertyDoChange(ConnectionChange, ConnectionChange::Disconnect, (SProperty*)input(), this);
    }
  }

void SProperty::connect(SProperty *prop) const
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

void SProperty::connect(const QVector<SProperty*> &l) const
  {
  if(l.size())
    {
    SBlock b(l.front()->handler());
    Q_FOREACH(SProperty *p, l)
      {
      connect(p);
      }
    }
  }

void SProperty::disconnect(SProperty *prop) const
  {
  xAssert(this == prop->input());
  prop->setInput(0);
  }

bool SProperty::isComputed() const
  {
  if(!isDynamic())
    {
    const SEmbeddedPropertyInstanceInformation *staticInfo = embeddedBaseInstanceInformation();
    return staticInfo->compute() != 0;
    }
  return false;
  }

void SProperty::disconnect() const
  {
  SProfileFunction
  if(_input)
    {
    ((SProperty*)_input)->disconnect((SProperty*)this);
    }

  while(_output)
    {
    disconnect(_output);
    }
  }

QVector<const SProperty *> SProperty::affects() const
  {
  QVector<SProperty *> aff = const_cast<SProperty*>(this)->affects();
  return *reinterpret_cast<QVector<const SProperty*>*>(&aff);
  }

QVector<SProperty *> SProperty::affects()
  {
  QVector<SProperty *> ret;

  const SEmbeddedPropertyInstanceInformation *info = embeddedBaseInstanceInformation();
  if(!info)
    {
    return ret;
    }

  xsize *affects = info->affects();
  if(!affects)
    {
    return ret;
    }

  SPropertyContainer *par = parent();
  const SPropertyInformation *parentInfo = par->typeInformation();
  while(*affects)
    {
    const SEmbeddedPropertyInstanceInformation *affected = parentInfo->child(*affects);

    ret << affected->locateProperty(par);
    affects++;
    }

  return ret;
  }

bool SProperty::ConnectionChange::apply()
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

bool SProperty::ConnectionChange::unApply()
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

bool SProperty::ConnectionChange::inform(bool back)
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

void SProperty::ConnectionChange::setParentHasInputConnection(SProperty *prop)
  {
  xAssert(prop);
  prop->_flags.setFlag(SProperty::ParentHasInput);

  SPropertyContainer *cont = prop->castTo<SPropertyContainer>();
  if(cont)
    {
    xForeach(auto child, cont->walker())
      {
      if(!child->_flags.hasFlag(SProperty::ParentHasInput))
        {
        child->_flags.setFlag(SProperty::ParentHasInput);
        setParentHasInputConnection(child);
        }
      }
    }
  }

void SProperty::ConnectionChange::setParentHasOutputConnection(SProperty *prop)
  {
  xAssert(prop);
  SPropertyContainer *cont = prop->castTo<SPropertyContainer>();
  if(cont)
    {
    xForeach(auto child, cont->walker())
      {
      if(!child->_flags.hasFlag(SProperty::ParentHasOutput))
        {
        child->_flags.setFlag(SProperty::ParentHasOutput);
        setParentHasOutputConnection(child);
        }
      }
    }
  }

void SProperty::ConnectionChange::clearParentHasInputConnection(SProperty *prop)
  {
  xAssert(prop);
  SPropertyContainer *cont = prop->castTo<SPropertyContainer>();
  if(cont)
    {
    if(!cont->input() &&
        (cont->isDynamic() ||
         !cont->embeddedBaseInstanceInformation()->isComputed() ) &&
        !cont->_flags.hasFlag(SProperty::ParentHasInput))
      {
      xForeach(auto child, cont->walker())
        {
        if(child->_flags.hasFlag(SProperty::ParentHasInput))
          {
          child->_flags.clearFlag(SProperty::ParentHasInput);
          clearParentHasInputConnection(child);
          }
        }
      }
    }
  }

void SProperty::ConnectionChange::clearParentHasOutputConnection(SProperty *prop)
  {
  xAssert(prop);
  SPropertyContainer *cont = prop->castTo<SPropertyContainer>();
  if(cont)
    {
    if(!cont->output() &&
       (cont->isDynamic() ||
        !cont->embeddedBaseInstanceInformation()->affectsSiblings() ) &&
        !cont->_flags.hasFlag(SProperty::ParentHasOutput))
      {
      xForeach(auto child, cont->walker())
        {
        if(child->_flags.hasFlag(SProperty::ParentHasOutput))
          {
          child->_flags.clearFlag(SProperty::ParentHasOutput);
          clearParentHasOutputConnection(child);
          }
        }
      }
    }
  }

void SProperty::connectInternal(SProperty *prop) const
  {
  // prop can't already have an output
  if(prop->hasInput())
    {
    return;
    }
  prop->_input = (SProperty*)this;

  SProperty **output = (SProperty**)&_output;
  while(*output)
    {
    SProperty **nextOp = &((*output)->_nextOutput);
    output = nextOp;
    }

  if(output)
    {
    *output = prop;
    }

#ifdef X_DEBUG
  const SProperty *p = this;
  while(p)
    {
    xAssert(p != p->nextOutput());
    p = p->nextOutput();
    }
#endif
  }

void SProperty::disconnectInternal(SProperty *prop) const
  {
  xAssert(prop->_input == this);

  prop->_input = 0;

  SProperty **output = (SProperty**)&_output;
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
  const SProperty *p = this;
  while(p)
    {
    xAssert(p != p->nextOutput());
    p = p->nextOutput();
    }
#endif
  }

QString SProperty::pathTo(const SProperty *that) const
  {
  return that->path(this);
  }

QString SProperty::path() const
  {
  SProfileFunction
  const SProperty *par = parent();
  if(par == 0)
    {
    return QString();
    }
  return par->path() + SDatabase::pathSeparator() + escapedName();
  }

QString SProperty::path(const SProperty *from) const
  {
  SProfileFunction

  if(from == this)
    {
    return "";
    }

  if(isDescendedFrom(from))
    {
    QString ret;
    const SProperty *p = parent();
    while(p && p != from)
      {
      xAssert(p->name() != "");
      ret = p->escapedName() + SDatabase::pathSeparator() + ret;

      p = p->parent();
      }
    return ret + escapedName();
    }

  const SProperty *parent = from->parent();
  if(parent)
    {
    return ".." + SDatabase::pathSeparator() + path(parent);
    }

  xAssert(0);
  return "";
  }

QString SProperty::mode() const
  {
  return baseInstanceInformation()->modeString();
  }

bool SProperty::isDescendedFrom(const SProperty *in) const
  {
  SProfileFunction
  if(this == in)
    {
    return true;
    }

  const SProperty *par = parent();
  if(par == 0)
    {
    return false;
    }
  return par->isDescendedFrom(in);
  }

SProperty *SProperty::resolvePath(const QString &path)
  {
  SProfileFunction
  preGet();

  SProperty *cur = this;

  QString name;
  bool escape = false;
  for(int i = 0, s = path.size(); i < s; ++i)
    {
    QChar c = path[i];

    if(c == QChar('\\'))
      {
      escape = true;
      }
    else
      {
      if(!escape && c != QChar('/'))
        {
        name.append(c);
        }

      if(!escape && (c == QChar('/') || i == (s-1)))
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
          SPropertyContainer* container = cur->castTo<SPropertyContainer>();
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

const SProperty *SProperty::resolvePath(const QString &path) const
  {
  return const_cast<SProperty*>(this)->resolvePath(path);
  }

QVariant SProperty::value() const
  {
  const SPropertyVariantInterface *varInt = interface<SPropertyVariantInterface>();

  if(varInt)
    {
    return varInt->asVariant(this);
    }
  return QString();
  }

void SProperty::setValue(const QVariant &val)
  {
  const SPropertyVariantInterface *varInt = interface<SPropertyVariantInterface>();

  if(varInt)
    {
    varInt->setVariant(this, val);
    }
  }

QString SProperty::valueAsString() const
  {
  const SPropertyVariantInterface *varInt = interface<SPropertyVariantInterface>();

  if(varInt)
    {
    return varInt->asString(this);
    }
  return QString();
  }

void SProperty::internalSetName(const QString &name)
  {
  ((BaseInstanceInformation*)this->baseInstanceInformation())->name() = name;
  }

void SProperty::postSet()
  {
  SProfileFunction
  setDependantsDirty();

  _flags.clearFlag(Dirty);
  }

void SProperty::setDirty()
  {
  SProfileFunction
  if(!_flags.hasAnyFlags(Dirty|PreGetting))
  {
    _flags.setFlag(Dirty);
    setDependantsDirty();

#ifdef S_CENTRAL_CHANGE_HANDLER
    SEntity *ent = entity();
    if(ent)
      {
      ent->informDirtyObservers(this);
      }
#endif
    }
  }

void SProperty::updateParent() const
  {
  const SProperty *par = parent();
  if(par)
    {
    par->preGet();
    }
  }

void SProperty::update() const
  {
  SProfileFunction
  SDatabase *db = const_cast<SDatabase*>(database());
  bool stateStorageEnabled = db->stateStorageEnabled();
  db->setStateStorageEnabled(false);

  SProperty *prop = const_cast<SProperty*>(this);
  prop->_flags.setFlag(PreGetting);

  // this is a const function, but because we delay computation we may need to assign here
  prop->_flags.clearFlag(Dirty);



  if(!isDynamic())
    {
    const EmbeddedInstanceInformation *child = embeddedBaseInstanceInformation();
    EmbeddedInstanceInformation::ComputeFunction compute = child->compute();
    if(compute)
      {
      SPropertyContainer *par = prop->embeddedParent();
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
void SProperty::addUserData(UserData *userData)
  {
  xAssert(userData);
  xAssert(!userData->next());
  if(userData && !userData->next())
    {
    userData->_next = _userData;
    _userData = userData;
    }
  }

void SProperty::removeUserData(UserData *userData)
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

SInterfaceBase *SProperty::interface(xuint32 typeId)
  {
  const SInterfaceBaseFactory* factory = typeInformation()->interfaceFactory(typeId);
  if(factory)
    {
    return const_cast<SInterfaceBaseFactory*>(factory)->classInterface(this);
    }
  return 0;
  }

const SInterfaceBase *SProperty::interface(xuint32 typeId) const
  {
  const SInterfaceBaseFactory* factory = typeInformation()->interfaceFactory(typeId);
  if(factory)
    {
    return const_cast<SInterfaceBaseFactory*>(factory)->classInterface(const_cast<SProperty*>(this));
    }
  return 0;

  }
