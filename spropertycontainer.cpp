#include "spropertycontainer.h"
#include "styperegistry.h"
#include "sdatabase.h"
#include "spropertyinformationhelpers.h"
#include "shandlerimpl.h"

S_IMPLEMENT_PROPERTY(SPropertyContainer, Shift)

void SPropertyContainer::createTypeInformation(SPropertyInformationTyped<SPropertyContainer> *info,
                                               const SPropertyInformationCreateData &data)
  {
  if(data.registerInterfaces)
    {
    auto *api = info->apiInterface();

    typedef XScript::XMethodToIndexedGetter<SPropertyContainer, SProperty *(xsize i), &SPropertyContainer::at> Getter;
    api->XInterfaceBase::setIndexAccessor(Getter::Get, Getter::GetDart);

    XInterfaceBase::NamedGetter namedGetter = XScript::XMethodToNamedGetter<SPropertyContainer, SProperty *(const QString &n), &SPropertyContainer::findChild>::Get;
    api->XInterfaceBase::setNamedAccessor(namedGetter);

    api->addProperty<xsize, &SPropertyContainer::size>("length");
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
    after()->internalInsertProperty(false, property(), _index);
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
    before()->internalInsertProperty(false, property(), _index);
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

SPropertyContainer::SPropertyContainer() : SProperty(), _child(0), _containedProperties(0)
  {
  }

xsize SPropertyContainer::size() const
  {
  preGet();
  xsize s = 0;
  SProperty *c = firstChild();
  while(c)
    {
    s++;
    c = nextSibling(c);
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
  for(SProperty *child=_child; child; child=child->_nextSibling)
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
  SProperty *prop = _child;
  while(prop)
    {
    if(child == prop)
      {
      return true;
      }
    prop = prop->_nextSibling;
    }
  return false;
  }

SPropertyContainer::~SPropertyContainer()
  {
  internalClear();
  }

void SPropertyContainer::clear()
  {
  SBlock b(handler());
  xAssert(handler());

  SProperty *prop = _child;
  while(prop)
    {
    xAssert(prop->parent() == this);
    SProperty *next = prop->_nextSibling;
    if(index(prop) >= _containedProperties)
      {
      removeProperty(prop);
      }
    prop = next;
    }
  _child = 0;
  }

void SPropertyContainer::internalClear()
  {
  xAssert(handler());

  SProperty *prop = _child;
  SProperty *previous = 0;
  while(prop)
    {
    SProperty *next = prop->_nextSibling;
    if(prop->isDynamic())
      {
      if(previous)
        {
        previous->_nextSibling = next;
        }
      else
        {
        _child = next;
        }
      database()->deleteDynamicProperty(prop);
      }
    else
      {
      database()->deleteProperty(prop);
      previous = prop;
      }
    prop = next;
    }
  _child = 0;
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

  handler()->doChange<TreeChange>((SPropertyContainer*)0, this, newProp, index);
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
    handler()->doChange<TreeChange>(this, c, p, X_SIZE_SENTINEL);
    }
  else
    {
    handler()->doChange<TreeChange>(this, c, p, X_SIZE_SENTINEL);
    }
  }

void SPropertyContainer::removeProperty(SProperty *oldProp)
  {
  xAssert(oldProp->parent() == this);

  SHandler* db = handler();
  xAssert(db);

  SBlock b(db);

  oldProp->disconnect();
  handler()->doChange<TreeChange>(this, (SPropertyContainer*)0, oldProp, index(oldProp));
  }

void SPropertyContainer::assignProperty(const SProperty *f, SProperty *t)
  {
  SProfileFunction
  const SPropertyContainer *from = f->uncheckedCastTo<SPropertyContainer>();
  SPropertyContainer *to = t->uncheckedCastTo<SPropertyContainer>();

  if(from->containedProperties() == to->containedProperties())
    {
    const SProperty *fChild=from->firstChild();
    SProperty *tChild=to->_child;
    xsize index = 0;
    while(fChild)
      {
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

      fChild = from->nextSibling(fChild);
      tChild = to->nextSibling(tChild);
      index++;
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

void SPropertyContainer::postChildSet(SPropertyContainer *cont, SProperty *p)
  {
  (void)cont;
  xAssert(cont->parent() || cont == cont->database());
  p->setDependantsDirty();
  }

void SPropertyContainer::internalInsertProperty(bool contained, SProperty *newProp, xsize index)
  {
  // xAssert(newProp->_entity == 0); may be true because of post init
  SPropertyInstanceInformation *newPropInstInfo = const_cast<SProperty::InstanceInformation*>(newProp->_instanceInfo);
  xAssert(newPropInstInfo->_dynamicParent == 0);
  xAssert(newProp->_nextSibling == 0);

  if(_child)
    {
    xsize propIndex = 0;
    SProperty *prop = _child;
    while(prop)
      {
      if((index == (propIndex+1) && index > _containedProperties) || !prop->_nextSibling)
        {
        if(contained)
          {
          xAssert(_containedProperties == (propIndex+1));
          xAssert(newPropInstInfo->_dynamicParent = 0);
          _containedProperties++;
          }
        else
          {
          newPropInstInfo->_index = propIndex + 1;
          newPropInstInfo->_dynamicParent = this;
          }
        // insert this prop into the list
        newProp->_nextSibling = prop->_nextSibling;
        prop->_nextSibling = newProp;

        // set up state info
        newProp->_handler = SHandler::findHandler(this, newProp);
        break;
        }
      propIndex++;
      prop = prop->_nextSibling;
      }
    }
  else
    {
    if(contained)
      {
      xAssert(_containedProperties == 0);
      xAssert(newPropInstInfo->_dynamicParent == 0);
      _containedProperties++;
      }
    else
      {
      newPropInstInfo->_index = 0;
      newPropInstInfo->_dynamicParent = this;
      }
    _child = newProp;
    newProp->_handler = SHandler::findHandler(this, newProp);
    }

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
  xAssert(oldProp->parent() == this);
  bool removed = false;

  if(oldProp == _child)
    {
    xAssert(_containedProperties == 0);

    _child = _child->_nextSibling;

    removed = true;
    ((SProperty::InstanceInformation*)oldProp->_instanceInfo)->_index = X_SIZE_SENTINEL;
    }
  else
    {
    xsize propIndex = 0;
    SProperty *prop = _child;
    while(prop)
      {
      if(oldProp == prop->_nextSibling)
        {
        xAssert((propIndex+1) >= _containedProperties);

        removed = true;
        ((SProperty::InstanceInformation*)oldProp->_instanceInfo)->_index = X_SIZE_SENTINEL;

        prop->_nextSibling = oldProp->_nextSibling;
        break;
        }
      propIndex++;
      prop = prop->_nextSibling;
      }
    }

  SProperty::ConnectionChange::clearParentHasInputConnection(oldProp);
  SProperty::ConnectionChange::clearParentHasOutputConnection(oldProp);

  xAssert(removed);
  SPropertyInstanceInformation *oldPropInstInfo = const_cast<SProperty::InstanceInformation*>(oldProp->_instanceInfo);
  // not dynamic or has a parent
  xAssert(!oldPropInstInfo->dynamic() || oldPropInstInfo->dynamicParent());
  oldPropInstInfo->_dynamicParent = 0;
  oldProp->_nextSibling = 0;
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


SProperty *SPropertyContainer::nextSibling(const SProperty *p) const
  {
  preGet();
  return p->_nextSibling;
  }

SProperty *SPropertyContainer::lastChild()
  {
  xForeach(auto child, walker())
    {
    if(!nextSibling(child))
      {
      return child;
      }
    }
  return 0;
  }


xsize SPropertyContainer::index(const SProperty* prop) const
  {
  SProfileFunction
  preGet();

  return prop->instanceInformation()->index();
  }
