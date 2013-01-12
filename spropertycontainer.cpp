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

void SPropertyContainer::disconnectTree()
  {
  disconnect();

  xForeach(auto p, walker())
    {
    SPropertyContainer *c = p->castTo<SPropertyContainer>();
    if(c)
      {
      c->disconnectTree();
      }
    else
      {
      p->disconnect();
      }
    }
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
  const SEmbeddedPropertyInstanceInformation *inst = typeInformation()->childFromName(name);
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
#ifdef S_CENTRAL_CHANGE_HANDLER
  xAssert(handler());
#endif
  SBlock b(handler());

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

void SPropertyContainer::internalClear(SDatabase *db)
  {
  SProperty *dynamic = _dynamicChild;
  while(dynamic)
    {
    SProperty *next = nextDynamicSibling(dynamic);
    db->deleteDynamicProperty(dynamic);
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
    ((SPropertyInstanceInformation*)newProp->_instanceInfo)->name() = makeUniqueName(name);
    }
  else
    {
    ((SPropertyInstanceInformation*)newProp->_instanceInfo)->name() = name;
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

#ifdef S_CENTRAL_CHANGE_HANDLER
  xAssert(db);
#endif

  SBlock b(db);

  SPropertyContainer *oldCont = oldProp->castTo<SPropertyContainer>();
  if(oldCont)
    {
    oldCont->disconnectTree();
    }
  else
    {
    oldProp->disconnect();
    }
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
  SDynamicPropertyInstanceInformation *newPropInstInfo =
      const_cast<SDynamicPropertyInstanceInformation*>(newProp->dynamicBaseInstanceInformation());
  xAssert(newPropInstInfo->parent() == 0);
  xAssert(newPropInstInfo->nextSibling() == 0);
  
  xsize propIndex = 0;

  if(_dynamicChild && index > 0)
    {
    xsize propIndex = containedProperties();
    SProperty *prop = _dynamicChild;
    while(prop)
      {
      SDynamicPropertyInstanceInformation *propInstInfo =
          const_cast<SDynamicPropertyInstanceInformation*>(prop->dynamicBaseInstanceInformation());

      if((index == (propIndex+1) && index > _containedProperties) ||
         !propInstInfo->nextSibling())
        {
        newPropInstInfo->setParent(this);

        propIndex = propInstInfo->index() + 1;

        // insert this prop into the list
        newPropInstInfo->setNextSibling(propInstInfo->nextSibling());
        propInstInfo->setNextSibling(newProp);
        break;
        }
      propIndex++;
      prop = nextDynamicSibling(prop);
      }
    }
  else
    {
    newPropInstInfo->setParent(this);

    if(_dynamicChild)
      {
      newPropInstInfo->setNextSibling(_dynamicChild);
      }
    
     propIndex = 0;

    _dynamicChild = newProp;
    }

  SDynamicPropertyInstanceInformation *propInstInfo =
      const_cast<SDynamicPropertyInstanceInformation*>(newProp->dynamicBaseInstanceInformation());
  while(propInstInfo)
    {
    newPropInstInfo->setIndex(propIndex);

    ++propIndex;

    if(!newPropInstInfo->nextSibling())
    {
      break;
    }

    newPropInstInfo =
        const_cast<SDynamicPropertyInstanceInformation*>(
          newPropInstInfo->nextSibling()->dynamicBaseInstanceInformation());
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

  if(output() || _flags.hasFlag(ParentHasOutput)) // || instanceInformation()->affectsSiblings())
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

  SDynamicPropertyInstanceInformation *oldPropInstInfo =
      const_cast<SDynamicPropertyInstanceInformation*>(oldProp->dynamicBaseInstanceInformation());

  xsize oldIndex = oldProp->baseInstanceInformation()->index();
  SDynamicPropertyInstanceInformation *indexUpdate = 0;

  if(oldProp == _dynamicChild)
    {
    xAssert(_containedProperties == 0);

    _dynamicChild = oldPropInstInfo->nextSibling();

    if(_dynamicChild)
      {
      indexUpdate =
          const_cast<SDynamicPropertyInstanceInformation*>(
            _dynamicChild->dynamicBaseInstanceInformation());
      }

    removed = true;
    oldPropInstInfo->setInvalidIndex();
    }
  else
    {
    xsize propIndex = 0;
    SProperty *prop = _dynamicChild;
    while(prop)
      {
      SDynamicPropertyInstanceInformation *propInstInfo =
          const_cast<SDynamicPropertyInstanceInformation*>(prop->dynamicBaseInstanceInformation());

      if(oldProp == propInstInfo->nextSibling())
        {
        xAssert((propIndex+1) >= _containedProperties);

        removed = true;
        oldPropInstInfo->setInvalidIndex();

        propInstInfo->setNextSibling(oldPropInstInfo->nextSibling());

        indexUpdate = propInstInfo;

        break;
        }
      propIndex++;
      prop = nextDynamicSibling(prop);
      }
    }

  while(indexUpdate)
    {
    indexUpdate->setIndex(oldIndex);

    ++oldIndex;

    if(!indexUpdate->nextSibling())
    {
      break;
    }

    indexUpdate =
        const_cast<SDynamicPropertyInstanceInformation*>(
          indexUpdate->nextSibling()->dynamicBaseInstanceInformation());
    }

  internalUnsetupProperty(oldProp);

  xAssert(removed);
  // not dynamic or has a parent
  xAssert(!oldPropInstInfo->isDynamic() || oldPropInstInfo->parent());
  oldPropInstInfo->setParent(0);
  oldPropInstInfo->setNextSibling(0);
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
  return p->dynamicBaseInstanceInformation()->nextSibling();
  }

const SProperty *SPropertyContainer::nextDynamicSibling(const SProperty *p) const
  {
  preGet();
  xAssert(p->isDynamic());
  return p->dynamicBaseInstanceInformation()->nextSibling();
  }

SProperty *SPropertyContainer::firstChild()
  {
  SEmbeddedPropertyInstanceInformation *inst = typeInformation()->firstChild();
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

  return prop->baseInstanceInformation()->index();
  }
