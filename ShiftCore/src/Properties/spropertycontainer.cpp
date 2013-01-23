#include "shift/Properties/spropertycontainer.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/sdatabase.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/Properties/spropertycontaineriterators.h"
#include "shift/Changes/shandler.inl"
#include "shift/Changes/spropertychanges.h"
#include "shift/sdatabase.h"
#include "XStringBuffer"

namespace Shift
{

S_IMPLEMENT_PROPERTY(PropertyContainer, Shift)

void PropertyContainer::createTypeInformation(PropertyInformationTyped<PropertyContainer> *info,
                                               const PropertyInformationCreateData &data)
  {
  if(data.registerInterfaces)
    {
    auto *api = info->apiInterface();

    //typedef XScript::XMethodToIndexedGetter<PropertyContainer, Property *(xsize i), &PropertyContainer::at> Getter;
    //api->XInterfaceBase::setIndexAccessor(Getter::Get, Getter::GetDart);

    //XInterfaceBase::NamedGetter namedGetter = XScript::XMethodToNamedGetter<PropertyContainer, Property *(const QString &n), &PropertyContainer::findChild>::Get;
    //api->XInterfaceBase::setNamedAccessor(namedGetter);

    static XScript::ClassDef<0,1,0> cls = {
      {
      api->property<xsize, &PropertyContainer::size>("length")
      }
    };

    api->buildInterface(cls);
    }
  }

PropertyContainer::TreeChange::TreeChange(PropertyContainer *b, PropertyContainer *a, Property *ent, xsize index)
  : _before(b), _after(a), _property(ent), _index(index), _owner(false)
  {
  }

PropertyContainer::TreeChange::~TreeChange()
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

bool PropertyContainer::TreeChange::apply()
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

bool PropertyContainer::TreeChange::unApply()
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

bool PropertyContainer::TreeChange::inform(bool back)
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

PropertyContainer::PropertyContainer()
  : Property(), _dynamicChild(0), _containedProperties(0)
#ifndef S_CENTRAL_CHANGE_HANDLER
    , _database(0)
#endif
  {
  }

xsize PropertyContainer::size() const
  {
  preGet();
  xsize s = typeInformation()->childCount();
  const Property *c = firstDynamicChild();
  while(c)
    {
    s++;
    c = nextDynamicSibling(c);
    }

  return s;
  }

void PropertyContainer::disconnectTree()
  {
  disconnect();

  xForeach(auto p, walker())
    {
    PropertyContainer *c = p->castTo<PropertyContainer>();
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

const Property *PropertyContainer::findChild(const PropertyNameArg &name) const
  {
  return const_cast<PropertyContainer*>(this)->findChild(name);
  }

Property *PropertyContainer::findChild(const PropertyNameArg &name)
  {
  preGet();
  return internalFindChild(name);
  }

Property *PropertyContainer::internalFindChild(const PropertyNameArg &name)
  {
  const EmbeddedPropertyInstanceInformation *inst = typeInformation()->childFromName(name);
  if(inst)
    {
    return inst->locateProperty(this);
    }

  for(Property *child=_dynamicChild; child; child=nextDynamicSibling(child))
    {
    if(name == child->name())
      {
      return child;
      }
    }
  return 0;
  }

const Property *PropertyContainer::internalFindChild(const PropertyNameArg &name) const
  {
  return const_cast<PropertyContainer*>(this)->internalFindChild(name);
  }

bool PropertyContainer::contains(const Property *child) const
  {
  preGet();
  return child->parent() == this;
  }

PropertyContainer::~PropertyContainer()
  {
  }

void PropertyContainer::clear()
{
#ifdef S_CENTRAL_CHANGE_HANDLER
  xAssert(handler());
#endif
  Block b(handler());

  Property *prop = _dynamicChild;
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

void PropertyContainer::internalClear(Database *db)
  {
  Property *dynamic = _dynamicChild;
  while(dynamic)
    {
    Property *next = nextDynamicSibling(dynamic);
    db->deleteDynamicProperty(dynamic);
    dynamic = next;
    }

  _dynamicChild = 0;
  }

PropertyName PropertyContainer::makeUniqueName(const PropertyNameArg &name) const
  {
  PropertyName newName;

  xuint32 id = 1;
  name.toName(newName);
  newName.appendType(id);
  while(internalFindChild(newName))
    {
    name.toName(newName);
    newName.appendType(id);

    ++id;
    }
  return newName;
  }

Property *PropertyContainer::addProperty(const PropertyInformation *info, xsize index, const PropertyNameArg& name, PropertyInstanceInformationInitialiser *init)
  {
  xAssert(index >= _containedProperties);


  Property *newProp = database()->createDynamicProperty(info, this, init);

  bool nameUnique = !name.isEmpty() && internalFindChild(name) == false;
  if(!nameUnique)
    {
    ((PropertyInstanceInformation*)newProp->_instanceInfo)->name() = makeUniqueName(name);
    }
  else
    {
    name.toName(((PropertyInstanceInformation*)newProp->_instanceInfo)->name());
    }

  PropertyDoChange(TreeChange, (PropertyContainer*)0, this, newProp, index);
  return newProp;
  }

void PropertyContainer::moveProperty(PropertyContainer *c, Property *p)
  {
  xAssert(p->parent() == this);

  const PropertyNameArg &name = p->name();
  bool nameUnique = c->internalFindChild(name) == false;
  if(!nameUnique)
    {
    Block b(database());

    p->setName(c->makeUniqueName(name));
    PropertyDoChange(TreeChange, this, c, p, X_SIZE_SENTINEL);
    }
  else
    {
    PropertyDoChange(TreeChange, this, c, p, X_SIZE_SENTINEL);
    }
  }

void PropertyContainer::removeProperty(Property *oldProp)
  {
  xAssert(oldProp->parent() == this);

  Handler* db = handler();

#ifdef S_CENTRAL_CHANGE_HANDLER
  xAssert(db);
#endif

  Block b(db);

  PropertyContainer *oldCont = oldProp->castTo<PropertyContainer>();
  if(oldCont)
    {
    oldCont->disconnectTree();
    }
  else
    {
    oldProp->disconnect();
    }
  PropertyDoChange(TreeChange, this, (PropertyContainer*)0, oldProp, index(oldProp));
  }

void PropertyContainer::internalInsertProperty(Property *newProp, xsize index)
  {
  // xAssert(newProp->_entity == 0); may be true because of post init
  DynamicPropertyInstanceInformation *newPropInstInfo =
      const_cast<DynamicPropertyInstanceInformation*>(newProp->dynamicBaseInstanceInformation());
  xAssert(newPropInstInfo->parent() == 0);
  xAssert(newPropInstInfo->nextSibling() == 0);

  xsize propIndex = 0;

  if(_dynamicChild && index > 0)
    {
    xsize propIndex = containedProperties();
    Property *prop = _dynamicChild;
    while(prop)
      {
      DynamicPropertyInstanceInformation *propInstInfo =
          const_cast<DynamicPropertyInstanceInformation*>(prop->dynamicBaseInstanceInformation());

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

  DynamicPropertyInstanceInformation *propInstInfo =
      const_cast<DynamicPropertyInstanceInformation*>(newProp->dynamicBaseInstanceInformation());
  while(propInstInfo)
    {
    newPropInstInfo->setIndex(propIndex);

    ++propIndex;

    if(!newPropInstInfo->nextSibling())
    {
      break;
    }

    newPropInstInfo =
        const_cast<DynamicPropertyInstanceInformation*>(
          newPropInstInfo->nextSibling()->dynamicBaseInstanceInformation());
    }

  internalSetupProperty(newProp);
  }

void PropertyContainer::internalSetupProperty(Property *newProp)
  {
  // set up state info
#ifdef S_CENTRAL_CHANGE_HANDLER
  newProp->_handler = Handler::findHandler(this, newProp);
#else
  PropertyContainer *cont = newProp->castTo<PropertyContainer>();
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

void PropertyContainer::internalRemoveProperty(Property *oldProp)
  {
  xAssert(oldProp);
  xAssert(oldProp->parent() == this);
  bool removed = false;

  DynamicPropertyInstanceInformation *oldPropInstInfo =
      const_cast<DynamicPropertyInstanceInformation*>(oldProp->dynamicBaseInstanceInformation());

  xsize oldIndex = oldProp->baseInstanceInformation()->index();
  DynamicPropertyInstanceInformation *indexUpdate = 0;

  if(oldProp == _dynamicChild)
    {
    xAssert(_containedProperties == 0);

    _dynamicChild = oldPropInstInfo->nextSibling();

    if(_dynamicChild)
      {
      indexUpdate =
          const_cast<DynamicPropertyInstanceInformation*>(
            _dynamicChild->dynamicBaseInstanceInformation());
      }

    removed = true;
    oldPropInstInfo->setInvalidIndex();
    }
  else
    {
    xsize propIndex = 0;
    Property *prop = _dynamicChild;
    while(prop)
      {
      DynamicPropertyInstanceInformation *propInstInfo =
          const_cast<DynamicPropertyInstanceInformation*>(prop->dynamicBaseInstanceInformation());

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
        const_cast<DynamicPropertyInstanceInformation*>(
          indexUpdate->nextSibling()->dynamicBaseInstanceInformation());
    }

  internalUnsetupProperty(oldProp);

  xAssert(removed);
  // not dynamic or has a parent
  xAssert(!oldPropInstInfo->isDynamic() || oldPropInstInfo->parent());
  oldPropInstInfo->setParent(0);
  oldPropInstInfo->setNextSibling(0);
  }

void PropertyContainer::internalUnsetupProperty(Property *oldProp)
  {
  Property::ConnectionChange::clearParentHasInputConnection(oldProp);
  Property::ConnectionChange::clearParentHasOutputConnection(oldProp);
  }

const Property *PropertyContainer::at(xsize i) const
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

Property *PropertyContainer::at(xsize i)
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

Property *PropertyContainer::nextDynamicSibling(const Property *p)
  {
  preGet();
  xAssert(p->isDynamic());
  return p->dynamicBaseInstanceInformation()->nextSibling();
  }

const Property *PropertyContainer::nextDynamicSibling(const Property *p) const
  {
  preGet();
  xAssert(p->isDynamic());
  return p->dynamicBaseInstanceInformation()->nextSibling();
  }

Property *PropertyContainer::firstChild()
  {
  const EmbeddedPropertyInstanceInformation *inst = typeInformation()->childFromIndex(0);
  if(inst)
    {
    return inst->locateProperty(this);
    }

  return firstDynamicChild();
  }

const Property *PropertyContainer::firstChild() const
  {
  return ((PropertyContainer*)this)->firstChild();
  }

Property *PropertyContainer::lastChild()
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

const Property *PropertyContainer::lastChild() const
  {
  return ((PropertyContainer*)this)->lastChild();
  }


xsize PropertyContainer::index(const Property* prop) const
  {
  SProfileFunction
  preGet();

  return prop->baseInstanceInformation()->index();
  }

}
