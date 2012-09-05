#include "spropertycontainer.h"
#include "styperegistry.h"
#include "sdatabase.h"
#include "spropertyinformationhelpers.h"
#include "spropertycontaineriterators.h"
#include "shandlerimpl.h"

S_IMPLEMENT_PROPERTY(SPropertyContainer, Shift)

void SPropertyContainer::createTypeInformation(SPropertyInformationTyped<SPropertyContainer> *info,
                                               const SPropertyInformationCreateData &data)
  {
  if(data.registerInterfaces)
    {
    auto *api = info->apiInterface();

    //typedef XScript::XMethodToIndexedGetter<SPropertyContainer, SProperty *(xsize i), &SPropertyContainer::at> Getter;
    //api->XInterfaceBase::setIndexAccessor(Getter::Get, Getter::GetDart);

    //XInterfaceBase::NamedGetter namedGetter = XScript::XMethodToNamedGetter<SPropertyContainer, SProperty *(const QString &n), &SPropertyContainer::findChild>::Get;
    //api->XInterfaceBase::setNamedAccessor(namedGetter);

    static XScript::ClassDef<0,1,0> cls = {
      {
      api->property<xsize, &SPropertyContainer::size>("length")
      }
    };

    api->buildInterface(cls);
    }
  }

SPropertyContainer::TreeChange::TreeChange(SPropertyContainer *b, SPropertyContainer *a, SProperty *ent, xsize index)
  : _before(b), _after(a), _property(ent), _index(index), _owner(false)
  {
  }

SPropertyContainer::TreeChange::~TreeChange()
  {
  if(_owner)
    {
    if(after(false))
      {
      after(false)->database()->deleteDynamicProperty(_property);
      }
    else if(before(false))
      {
      before(false)->database()->deleteDynamicProperty(_property);
      }
    else
      {
      xAssert(0 && "No parents?");
      }
    }
  }

bool SPropertyContainer::TreeChange::apply()
  {
  SProfileFunction
  if(before(false))
    {
    _owner = true;
    before()->internalRemoveProperty(property());
    before()->postSet();
    }

  if(after(false))
    {
    _owner = false;
    after()->internalInsertProperty(property(), _index);
    after()->postSet();
    }
  return true;
  }

bool SPropertyContainer::TreeChange::unApply()
  {
  SProfileFunction
  if(after())
    {
    _owner = true;
    after()->internalRemoveProperty(property());
    after()->postSet();
    }

  if(before())
    {
    _owner = false;
    before()->internalInsertProperty(property(), _index);
    before()->postSet();
    }
  return true;
  }

bool SPropertyContainer::TreeChange::inform(bool back)
  {
  SProfileFunction
  if(after() && (!before() || !before()->isDescendedFrom(after())))
    {
    xAssert(property()->entity());
    property()->entity()->informTreeObservers(this, back);
    }

  if(before() && (!after() || !after()->isDescendedFrom(before())))
    {
    before()->entity()->informTreeObservers(this, back);
    }
  return true;
  }

SPropertyContainer::SPropertyContainer()
  : SProperty(), _dynamicChild(0), _containedProperties(0)
#ifndef S_CENTRAL_CHANGE_HANDLER
    , _database(0)
#endif
  {
  }

xsize SPropertyContainer::size() const
  {
  preGet();
  xsize s = typeInformation()->childCount();
  const SProperty *c = firstDynamicChild();
  while(c)
    {
    s++;
    c = nextDynamicSibling(c);
    }

  return s;
  }

const SProperty *SPropertyContainer::findChild(const QString &name) const
  {
  return const_cast<SPropertyContainer*>(this)->findChild(name);
  }

SProperty *SPropertyContainer::findChild(const QString &name)
  {
  preGet();
  return internalFindChild(name);
  }

SProperty *SPropertyContainer::internalFindChild(const QString &name)
  {
  const SPropertyInstanceInformation *inst = typeInformation()->childFromName(name);
  if(inst)
    {
    return inst->locateProperty(this);
    }

  for(SProperty *child=_dynamicChild; child; child=nextDynamicSibling(child))
    {
    if(child->name() == name)
      {
      return child;
      }
    }
  return 0;
  }

const SProperty *SPropertyContainer::internalFindChild(const QString &name) const
  {
  return const_cast<SPropertyContainer*>(this)->internalFindChild(name);
  }

bool SPropertyContainer::contains(const SProperty *child) const
  {
  preGet();
  return child->parent() == this;
  }

SPropertyContainer::~SPropertyContainer()
  {
  }

void SPropertyContainer::clear()
  {
  SBlock b(handler());
  xAssert(handler());

  SProperty *prop = _dynamicChild;
  while(prop)
    {
    xAssert(prop->parent() == this);
    if(index(prop) >= _containedProperties)
      {
      removeProperty(prop);
      }
    prop = _dynamicChild;
    }

  xAssert(_dynamicChild == 0);
  }

void SPropertyContainer::internalClear()
  {
#ifdef S_CENTRAL_CHANGE_HANDLER
  xAssert(handler());
#endif

  SProperty *dynamic = _dynamicChild;
  while(dynamic)
    {
    SProperty *next = nextDynamicSibling(dynamic);
    database()->deleteDynamicProperty(dynamic);
    dynamic = next;
    }

  _dynamicChild = 0;
  }

QString SPropertyContainer::makeUniqueName(const QString &name) const
  {
  QString newName;

  xuint32 id = 1;
  newName = name + QString::number(id);
  while(internalFindChild(newName))
    {
    newName = name + QString::number(id);
    ++id;
    }
  return newName;
  }

SProperty *SPropertyContainer::addProperty(const SPropertyInformation *info, xsize index, const QString& name, SPropertyInstanceInformationInitialiser *init)
  {
  xAssert(index >= _containedProperties);


  SProperty *newProp = database()->createDynamicProperty(info, this, init);

  bool nameUnique = !name.isEmpty() && internalFindChild(name) == false;
  if(!nameUnique)
    {
    ((SPropertyInstanceInformation*)newProp->_instanceInfo)->_name = makeUniqueName(name);
    }
  else
    {
    ((SPropertyInstanceInformation*)newProp->_instanceInfo)->_name = name;
    }

  SPropertyDoChange(TreeChange, (SPropertyContainer*)0, this, newProp, index);
  return newProp;
  }

void SPropertyContainer::moveProperty(SPropertyContainer *c, SProperty *p)
  {
  xAssert(p->parent() == this);

  const QString &name = p->name();
  bool nameUnique = c->internalFindChild(name) == false;
  if(!nameUnique)
    {
    SBlock b(database());

    p->setName(c->makeUniqueName(name));
    SPropertyDoChange(TreeChange, this, c, p, X_SIZE_SENTINEL);
    }
  else
    {
    SPropertyDoChange(TreeChange, this, c, p, X_SIZE_SENTINEL);
    }
  }

void SPropertyContainer::removeProperty(SProperty *oldProp)
  {
  xAssert(oldProp->parent() == this);

  SHandler* db = handler();
  xAssert(db);

  SBlock b(db);

  oldProp->disconnect();
  SPropertyDoChange(TreeChange, this, (SPropertyContainer*)0, oldProp, index(oldProp));
  }

void SPropertyContainer::assignProperty(const SProperty *f, SProperty *t)
  {
  SProfileFunction
  const SPropertyContainer *from = f->uncheckedCastTo<SPropertyContainer>();
  SPropertyContainer *to = t->uncheckedCastTo<SPropertyContainer>();

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

void SPropertyContainer::saveProperty(const SProperty *p, SSaver &l)
  {
  SProfileFunction
  const SPropertyContainer *c = p->uncheckedCastTo<SPropertyContainer>();
  xAssert(c);

  SProperty::saveProperty(p, l);

  l.saveChildren(c);
  }

SProperty *SPropertyContainer::loadProperty(SPropertyContainer *parent, SLoader &l)
  {
  SProfileFunction
  xAssert(parent);

  SProperty *prop = SProperty::loadProperty(parent, l);
  xAssert(prop);

  SPropertyContainer* container = prop->uncheckedCastTo<SPropertyContainer>();

  l.loadChildren(container);

  return prop;
  }

bool SPropertyContainer::shouldSavePropertyValue(const SProperty *p)
  {
  const SPropertyContainer *ptr = p->uncheckedCastTo<SPropertyContainer>();
  if(ptr->_containedProperties < ptr->size())
    {
    return true;
    }

  xForeach(auto p, ptr->walker())
    {
    const SPropertyInformation *info = p->typeInformation();
    if(info->functions().shouldSave(p))
      {
      return true;
      }
    }

  return false;
  }

void SPropertyContainer::internalInsertProperty(SProperty *newProp, xsize index)
  {
  // xAssert(newProp->_entity == 0); may be true because of post init
  SPropertyInstanceInformation *newPropInstInfo = const_cast<SProperty::InstanceInformation*>(newProp->_instanceInfo);
  xAssert(newPropInstInfo->_dynamicParent == 0);
  xAssert(newPropInstInfo->_dynamicNextSibling == 0);

  if(_dynamicChild)
    {
    xsize propIndex = 0;
    SProperty *prop = _dynamicChild;
    while(prop)
      {
      SPropertyInstanceInformation *propInstInfo = const_cast<SProperty::InstanceInformation*>(prop->_instanceInfo);

      if((index == (propIndex+1) && index > _containedProperties) || !propInstInfo->dynamicNextSibling())
        {
        newPropInstInfo->_index = propIndex + 1;
        newPropInstInfo->_dynamicParent = this;

        // insert this prop into the list
        newPropInstInfo->_dynamicNextSibling = propInstInfo->_dynamicNextSibling;
        propInstInfo->_dynamicNextSibling = newProp;
        break;
        }
      propIndex++;
      prop = nextDynamicSibling(prop);
      }
    }
  else
    {
    newPropInstInfo->_index = 0;
    newPropInstInfo->_dynamicParent = this;

    xAssert(newPropInstInfo->dynamicNextSibling() == 0);

    _dynamicChild = newProp;
    }

  internalSetupProperty(newProp);
  }

void SPropertyContainer::internalSetupProperty(SProperty *newProp)
  {
  // set up state info
#ifdef S_CENTRAL_CHANGE_HANDLER
  newProp->_handler = SHandler::findHandler(this, newProp);
#else
  SPropertyContainer *cont = newProp->castTo<SPropertyContainer>();
  if(cont)
    {
    xAssert(_database);
    cont->_database = _database;
    }
#endif

  // is any prop in
  bool parentComputed = isComputed() || _flags.hasFlag(ParentHasInput);
  if(input() || _flags.hasFlag(ParentHasInput) || parentComputed)
    {
    newProp->_flags.setFlag(ParentHasInput);
    }

  if(output() || _flags.hasFlag(ParentHasOutput) || instanceInformation()->affectsSiblings())
    {
    newProp->_flags.setFlag(ParentHasOutput);
    }
  xAssert(newProp->parent());
  }

void SPropertyContainer::internalRemoveProperty(SProperty *oldProp)
  {
  xAssert(oldProp);
  xAssert(oldProp->parent() == this);
  bool removed = false;

  if(oldProp == _dynamicChild)
    {
    xAssert(_containedProperties == 0);

    SPropertyInstanceInformation *propInstInfo = const_cast<SProperty::InstanceInformation*>(oldProp->_instanceInfo);

    _dynamicChild = propInstInfo->_dynamicNextSibling;

    removed = true;
    propInstInfo->_index = X_SIZE_SENTINEL;
    }
  else
    {
    xsize propIndex = 0;
    SProperty *prop = _dynamicChild;
    while(prop)
      {
      SPropertyInstanceInformation *propInstInfo = const_cast<SProperty::InstanceInformation*>(prop->_instanceInfo);

      if(oldProp == propInstInfo->_dynamicNextSibling)
        {
        xAssert((propIndex+1) >= _containedProperties);

        SPropertyInstanceInformation *oldPropInstInfo = const_cast<SProperty::InstanceInformation*>(oldProp->_instanceInfo);

        removed = true;
        oldPropInstInfo->_index = X_SIZE_SENTINEL;

        propInstInfo->_dynamicNextSibling = oldPropInstInfo->_dynamicNextSibling;
        break;
        }
      propIndex++;
      prop = nextDynamicSibling(prop);
      }
    }

  internalUnsetupProperty(oldProp);

  xAssert(removed);
  SPropertyInstanceInformation *oldPropInstInfo = const_cast<SProperty::InstanceInformation*>(oldProp->_instanceInfo);
  // not dynamic or has a parent
  xAssert(!oldPropInstInfo->dynamic() || oldPropInstInfo->dynamicParent());
  oldPropInstInfo->_dynamicParent = 0;
  oldPropInstInfo->_dynamicNextSibling = 0;
  }

void SPropertyContainer::internalUnsetupProperty(SProperty *oldProp)
  {
  SProperty::ConnectionChange::clearParentHasInputConnection(oldProp);
  SProperty::ConnectionChange::clearParentHasOutputConnection(oldProp);
  }

const SProperty *SPropertyContainer::at(xsize i) const
  {
  xForeach(auto x, walker())
    {
    if(!i)
      {
      return x;
      }
    --i;
    }

  return 0;
  }

SProperty *SPropertyContainer::at(xsize i)
  {
  xForeach(auto x, walker())
    {
    if(!i)
      {
      return x;
      }
    --i;
    }

  return 0;
  }

SProperty *SPropertyContainer::nextDynamicSibling(const SProperty *p)
  {
  preGet();
  xAssert(p->isDynamic());
  return p->instanceInformation()->dynamicNextSibling();
  }

const SProperty *SPropertyContainer::nextDynamicSibling(const SProperty *p) const
  {
  preGet();
  xAssert(p->isDynamic());
  return p->instanceInformation()->dynamicNextSibling();
  }

SProperty *SPropertyContainer::firstChild()
  {
  SPropertyInstanceInformation *inst = typeInformation()->firstChild();
  if(inst)
    {
    return inst->locateProperty(this);
    }

  return firstDynamicChild();
  }

const SProperty *SPropertyContainer::firstChild() const
  {
  return ((SPropertyContainer*)this)->firstChild();
  }

SProperty *SPropertyContainer::lastChild()
  {
  xForeach(auto child, walker())
    {
    if(!nextDynamicSibling(child))
      {
      return child;
      }
    }
  return 0;
  }

const SProperty *SPropertyContainer::lastChild() const
  {
  return ((SPropertyContainer*)this)->lastChild();
  }


xsize SPropertyContainer::index(const SProperty* prop) const
  {
  SProfileFunction
  preGet();

  return prop->instanceInformation()->index();
  }
