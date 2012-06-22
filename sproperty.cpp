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
    api->addProperty<const SPropertyInformation *, &SProperty::typeInformation>("typeInformation");
    api->addProperty<SProperty *, const SProperty *, &SProperty::input, &SProperty::setInput>("input");
    api->addProperty<SPropertyContainer *, SPropertyContainer *, &SProperty::parent, &SProperty::setParent>("parent");

    api->addProperty<SProperty *, &SProperty::output>("firstOutput");
    api->addProperty<SProperty *, &SProperty::nextOutput>("nextOutput");

    api->addProperty<QString, &SProperty::mode>("mode");
    api->addProperty<bool, &SProperty::isDynamic>("dynamic");
    api->addProperty<const QString &, const QString &, &SProperty::name, &SProperty::setName>("name");

    api->addProperty<QVariant, const QVariant &, &SProperty::value, &SProperty::setValue>("value");
    api->addProperty<QString, &SProperty::valueAsString>("valueString");

    api->addProperty<QVector<SProperty*>, &SProperty::affects>("affects");

    api->addConstMethod<QString (const SProperty *), &SProperty::pathTo>("pathTo");

    api->addMethod<void(), &SProperty::beginBlock>("beginBlock");
    api->addMethod<void(bool), &SProperty::endBlock>("endBlock");

    api->addConstMethod<bool(const SProperty *), &SProperty::equals>("equals");

    info->addStaticInterface(new SBasicColourInterface);
    }
  }

void SProperty::setDependantsDirty()
  {
  for(SProperty *o=output(); o; o = o->nextOutput())
    {
    o->setDirty();
    }

  const SPropertyInstanceInformation *child = baseInstanceInformation();

  if(child && child->affects())
    {
    SPropertyContainer *par = parent();
    const SPropertyInformation *parentInfo = par->typeInformation();
    xsize i=0;
    while(child->affects()[i])
      {
      const SPropertyInstanceInformation *propInst = parentInfo->child(child->affects()[i]);
      SProperty *affectsProp = propInst->locateProperty(par);

      xAssert(affectsProp);
      affectsProp->setDirty();
      i++;
      }
    }

  // ?  || isComputed()
  if(input() || _flags.hasFlag(SProperty::ParentHasInput))
    {
    SPropertyContainer *c = castTo<SPropertyContainer>();
    if(c)
      {
      SProperty *child = c->_child;
      while(child)
        {
        child->setDirty();
        child = child->_nextSibling;
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

SProperty::SProperty() : _nextSibling(0), _input(0), _output(0), _nextOutput(0),
    _handler(0), _parent(0), _instanceInfo(0), _entity(0),
    _flags(Dirty)

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

SProperty::~SProperty()
  {
#ifdef S_PROPERTY_USER_DATA
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
#endif

  if(isDynamic())
    {
    ((InstanceInformation*)instanceInformation())->~InstanceInformation();
    }
  }

bool SProperty::isDynamic() const
  {
  SProfileFunction
  return instanceInformation()->dynamic();
  }

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

  handler()->doChange<NameChange>(name(), realName, this);
  }

void SProperty::assignProperty(const SProperty *, SProperty *)
  {
  }

void SProperty::saveProperty(const SProperty *p, SSaver &l)
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

    const SPropertyInstanceInformation *instInfo = p->instanceInformation();

    if(!instInfo->isDefaultMode())
      {
      QString mode = instInfo->modeString();

      l.beginAttribute("mode");
      writeValue(l, mode);
      l.endAttribute("mode");
      }

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
    }

  if(p->input())
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
    Initialiser() : affects(0) { }
    void initialise(SPropertyInstanceInformation *inst)
      {
      inst->setAffects(affects);
      inst->setModeString(mode);
      }

    xsize *affects;
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

SDatabase *SProperty::database()
  {
  return handler()->database();
  }

const SDatabase *SProperty::database() const
  {
  return handler()->database();
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
    xsize inputLocation = p->instanceInformation()->defaultInput();
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

const XInterfaceBase *SProperty::apiInterface() const
  {
  return typeInformation()->apiInterface();
  }

const XInterfaceBase *SProperty::staticApiInterface()
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

SEntity *SProperty::entity() const
  {
  SProfileFunction
  if(_entity)
    {
    return _entity;
    }
  _entity = (SEntity *)castTo<SEntity>();
  if(_entity)
    {
    return _entity;
    }
  if(_parent)
    {
    _entity = _parent->entity();
    }
  return _entity;
  }

void SProperty::setParent(SPropertyContainer *newParent)
  {
  parent()->moveProperty(newParent, this);
  }

void SProperty::connect(SProperty *prop) const
  {
  SProfileFunction
  if(prop && prop != this)
    {
    ((SDatabase*)handler())->doChange<ConnectionChange>(ConnectionChange::Connect, (SProperty*)this, prop);
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
  SProfileFunction
  ((SDatabase*)handler())->doChange<ConnectionChange>(ConnectionChange::Disconnect, (SProperty*)this, prop);
  }

bool SProperty::isComputed() const
  {
  return baseInstanceInformation()->compute() != 0;
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

QVector<SProperty *> SProperty::affects() const
  {
  QVector<SProperty *> ret;

  const SPropertyInstanceInformation *info = instanceInformation();
  xsize *affects = info->affects();
  if(!affects)
    {
    return ret;
    }

  const SPropertyInformation *parentInfo = parent()->typeInformation();
  while(*affects)
    {
    const SPropertyInstanceInformation *affected = parentInfo->child(*affects);

    ret << affected->locateProperty(parent());
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
  prop->_flags.setFlag(ParentHasInput);

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
    SProperty *child = cont->_child;
    while(child)
      {
      if(!child->_flags.hasFlag(SProperty::ParentHasOutput))
        {
        child->_flags.setFlag(SProperty::ParentHasOutput);
        setParentHasOutputConnection(child);
        }
      child = child->_nextSibling;
      }
    }
  }

void SProperty::ConnectionChange::clearParentHasInputConnection(SProperty *prop)
  {
  xAssert(prop);
  SPropertyContainer *cont = prop->castTo<SPropertyContainer>();
  if(cont)
    {
    xForeach(auto child, cont->walker())
      {
      if(child->_flags.hasFlag(SProperty::ParentHasInput) &&
         (!prop->parent()->input() &&
          !prop->parent()->instanceInformation()->isComputed() &&
          !prop->parent()->_flags.hasFlag(ParentHasInput)))
        {
        child->_flags.clearFlag(SProperty::ParentHasInput);
        clearParentHasInputConnection(child);
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
    xForeach(auto child, cont->walker())
      {
      if(child->_flags.hasFlag(SProperty::ParentHasOutput) &&
         (!prop->parent()->output() &&
          !prop->parent()->instanceInformation()->affectsSiblings() &&
          !prop->parent()->_flags.hasFlag(ParentHasOutput)))
        {
        child->_flags.clearFlag(SProperty::ParentHasOutput);
        clearParentHasOutputConnection(child);
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
    output = &((*output)->_nextOutput);
    }

  if(output)
    {
    *output = prop;
    }
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
      return;
      }
    else
      {
      output = &((*output)->_nextOutput);
      }
    }
  }

QString SProperty::path() const
  {
  SProfileFunction
  if(parent() == 0)
    {
    return QString();
    }
  return parent()->path() + SDatabase::pathSeparator() + escapedName();
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
  else if(from->parent())
    {
    return ".." + SDatabase::pathSeparator() + path(from->parent());
    }
  xAssert(0);
  return "";
  }

QString SProperty::mode() const
  {
  return instanceInformation()->modeString();
  }

bool SProperty::isDescendedFrom(const SProperty *in) const
  {
  SProfileFunction
  if(this == in)
    {
    return true;
    }
  else if(parent() == 0)
    {
    return false;
    }
  return parent()->isDescendedFrom(in);
  }

SProperty *SProperty::resolvePath(const QString &path)
  {
  SProfileFunction
  preGet();

  SProperty *cur = this;

  QString name;
  bool escape = false;
  for(xsize i = 0, s = path.size(); i < s; ++i)
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
  ((InstanceInformation*)this->baseInstanceInformation())->_name = name;
  }

void SProperty::postSet()
  {
  SProfileFunction
  SPropertyContainer *c = parent();
  xAssert(c); // setting the db object? that is odd. Or a property in undo somewhere?
  const SPropertyInformation *info = c->typeInformation();
  info->functions().postChildSet(c, this);

  _flags.clearFlag(Dirty);
  }

void SProperty::setDirty()
  {
  SProfileFunction
  if((!_flags.hasAnyFlags(Dirty|PreGetting)))
  {
    _flags.setFlag(Dirty);
    SPropertyContainer *c = parent();
    xAssert(c);
    const SPropertyInformation *info = c->typeInformation();
    info->functions().postChildSet(c, this);

    if(entity())
      {
      entity()->informDirtyObservers(this);
      }
    }
  }

void SProperty::updateParent() const
  {
  if(parent())
    {
    parent()->preGet();
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

  const SPropertyInstanceInformation *child = baseInstanceInformation();
  if(child && child->compute())
    {
    xAssert(parent());
    SProcessManager::preCompute(child, parent());
    child->compute()(child, parent());
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
