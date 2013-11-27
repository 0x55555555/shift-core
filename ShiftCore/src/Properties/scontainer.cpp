#include "shift/Properties/scontainer.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/sdatabase.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/Properties/scontaineriterators.h"
#include "shift/Changes/shandler.inl"
#include "shift/Changes/spropertychanges.h"
#include "shift/sdatabase.h"
#include "shift/Utilities/siterator.h"
#include "shift/Properties/scontainerinternaliterators.h"
#include "XStringBuffer"
#include "XIntrusiveLinkedList"

namespace Shift
{

void disconnectHelper(Attribute *a)
  {
  if(Container *c = a->castTo<Container>())
    {
    c->disconnectTree();
    }
  else if(Property *prop = a->castTo<Property>())
    {
    prop->disconnect();
    }
  }

S_IMPLEMENT_PROPERTY(Container, Shift)

void Container::createTypeInformation(PropertyInformationTyped<Container> *info,
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
      api->property<xsize, &Container::size>("length")
      }
    };

    api->buildInterface(cls);
    }
  }

Container::TreeChange::TreeChange(Container *b, Container *a, Attribute *ent, xsize index)
  : _before(b), _after(a), _attribute(ent), _index(index), _owner(false)
  {
  }

Container::TreeChange::~TreeChange()
  {
  if(_owner)
    {
    if(after(false))
      {
      after(false)->database()->deleteDynamicAttribute(_attribute);
      }
    else if(before(false))
      {
      before(false)->database()->deleteDynamicAttribute(_attribute);
      }
    else
      {
      xAssert(0 && "No parents?");
      }
    }
  }

bool Container::TreeChange::apply()
  {
  SProfileFunction

  // its possible the tree is computed, but we are trying to insert into it.
  // its also possible this node is part of a ParentHasInput chain.
  // either way, rather than post setting this node, leaving a gaping hole in
  // the dirty chain, we pre get and post set, ensuring dirty flags are correct.

  if(before(false))
    {
    _owner = true;
    before()->preGet();
    before()->internalRemove(property());
    before()->postSet();
    }

  if(after(false))
    {
    _owner = false;
    after()->preGet();
    after()->internalInsert(property(), _index);
    after()->postSet();
    }
  return true;
  }

bool Container::TreeChange::unApply()
  {
  SProfileFunction
  if(after())
    {
    _owner = true;
    after()->internalRemove(property());
    after()->postSet();
    }

  if(before())
    {
    _owner = false;
    before()->internalInsert(property(), _index);
    before()->postSet();
    }
  return true;
  }

bool Container::TreeChange::inform(bool back)
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

Container::Container()
  : Property(), _dynamicChild(0)
  {
  }

xsize Container::size() const
  {
  preGet();
  xsize s = typeInformation()->childCount();
  const Attribute *c = firstDynamicChild();
  while(c)
    {
    s++;
    c = nextDynamicSibling(c);
    }

  return s;
  }

xuint8 Container::containedProperties() const
  {
  return typeInformation()->childCount();
  }

void Container::disconnectTree()
  {
  disconnect();

  xForeach(auto p, LightWalker(this))
    {
    disconnectHelper(p);
    }
  }

const Attribute *Container::findChild(const NameArg &name) const
  {
  return const_cast<Container*>(this)->findChild(name);
  }

Attribute *Container::findChild(const NameArg &name)
  {
  preGet();
  return internalFindChild(name);
  }

Attribute *Container::internalFindChild(const NameArg &name)
  {
  const EmbeddedPropertyInstanceInformation *inst = typeInformation()->childFromName(name);
  if(inst)
    {
    return inst->locate(this);
    }

  for(Attribute *child = _dynamicChild; child; child = nextDynamicSibling(child))
    {
    if(name == child->name())
      {
      return child;
      }
    }
  return 0;
  }

const Attribute *Container::internalFindChild(const NameArg &name) const
  {
  return const_cast<Container*>(this)->internalFindChild(name);
  }

bool Container::contains(const Attribute *child) const
  {
  preGet();
  return child->parent() == this;
  }

Container::~Container()
  {
  }

void Container::clear()
  {
#ifdef S_CENTRAL_CHANGE_HANDLER
  xAssert(handler());
#endif
  Block b(handler());

  xuint8 containedProps = containedProperties();

  Attribute *prop = _dynamicChild;
  while(prop)
    {
    xAssert(prop->parent() == this);
    if(index(prop) >= containedProps)
      {
      removeAttribute(prop);
      }
    prop = _dynamicChild;
    }

  xAssert(_dynamicChild == 0);
  }

void Container::internalClear(Database *db)
  {
  Attribute *dynamic = _dynamicChild;
  while(dynamic)
    {
    Attribute *next = nextDynamicSibling(dynamic);
    disconnectHelper(dynamic);
    db->deleteDynamicAttribute(dynamic);
    dynamic = next;
    }

  _dynamicChild = 0;
  }

void Container::makeUniqueName(
    const Attribute *prop,
    const NameArg &name,
    Name& newName) const
  {
  const PropertyInformation *info = prop->typeInformation();
  NameArg typeWrap(info->typeName());
  const NameArg *inName = name.isEmpty() ? &typeWrap : &name;

  xsize id = info->instances();
  inName->toName(newName);
  newName.appendType(id);
  while(internalFindChild(newName))
    {
    inName->toName(newName);
    newName.appendType(id);

    ++id;
    }
  }

Attribute *Container::addAttribute(const PropertyInformation *info, xsize index, const NameArg& name, PropertyInstanceInformationInitialiser *init)
  {
  xAssert(index >= containedProperties());
  Attribute *newProp = database()->addDynamicAttribute(info, name, index, this, init);
  return newProp;
  }

void Container::moveAttribute(Container *c, Attribute *p)
  {
  xAssert(p->parent() == this);

  const NameArg &name = p->name();
  bool nameUnique = c->internalFindChild(name) == false;
  if(!nameUnique)
    {
    Block b(database());

    Name newName;
    c->makeUniqueName(p, name, newName);
    p->setName(newName);
    PropertyDoChange(TreeChange, this, c, p, X_UINT8_SENTINEL);
    }
  else
    {
    PropertyDoChange(TreeChange, this, c, p, X_UINT8_SENTINEL);
    }
  }

void Container::removeAttribute(Attribute *oldProp)
  {
  xAssert(oldProp && oldProp->parent() == this);

  Handler* db = handler();

#ifdef S_CENTRAL_CHANGE_HANDLER
  xAssert(db);
#endif

  Block b(db);

#if X_ASSERTS_ENABLED
  Iterator::ChildTree it;
  it.reset(oldProp);
  xForeach(auto a, it)
    {
    if(Property *prop = a->castTo<Property>())
      {
      xAssert(!prop->isUpdating());
      }
    }
#endif

  disconnectHelper(oldProp);
  PropertyDoChange(TreeChange, this, (Container*)0, oldProp, index(oldProp));
  }

class ChildLL : public Eks::IntrusiveLinkedListBase<Attribute, ChildLL>
  {
public:
  static Attribute **getNextLocation(Attribute *prev)
    {
    xAssert(prev);
    auto instInfo = getInstanceInfo(prev);

    return &instInfo->_nextSibling;
    }
  static const Attribute *const *getNextLocation(const Attribute *prev)
    {
    const DynamicPropertyInstanceInformation *instInfo = prev->dynamicBaseInstanceInformation();

    return &instInfo->_nextSibling;
    }

  static DynamicPropertyInstanceInformation *getInstanceInfo(Attribute *prop)
    {
    xAssert(prop);
    DynamicPropertyInstanceInformation *instInfo =
        const_cast<DynamicPropertyInstanceInformation*>(prop->dynamicBaseInstanceInformation());
    return instInfo;
    }
  };

void Container::internalInsert(Attribute *newProp, xsize index)
  {
  preGet();

  // setup the new prop's instance info
    {
    auto newPropInstInfo = ChildLL::getInstanceInfo(newProp);
    xAssert(newPropInstInfo->parent() == 0);
    xAssert(newPropInstInfo->nextSibling() == 0);

    newPropInstInfo->setParent(this);
    }

  xsize insertedIndex = index;
  xsize dynamicIndex = index - containedProperties();


  Attribute *justBefore = nullptr;
  if (index != X_SIZE_SENTINEL)
    {
    justBefore = ChildLL::insert(&_dynamicChild, newProp, dynamicIndex);

    if (!justBefore && dynamicIndex != 0)
      {
      xAssertFail();
      return;
      }
    }
  else
    {
    justBefore = ChildLL::append(&_dynamicChild, newProp, &insertedIndex);
    }

  Attribute *after = newProp;
  while(after)
    {
    auto newPropInstInfo = ChildLL::getInstanceInfo(after);

    newPropInstInfo->setIndex(insertedIndex++);
    after = ChildLL::getNext(after);
    }

  internalSetup(newProp);
  }

void Container::internalSetup(Attribute *newProp)
  {
  // set up state info
#ifdef S_CENTRAL_CHANGE_HANDLER
  if (Entity *ent = newProp->castTo<Entity>())
    {
    ent->_handler = Handler::findHandler(this, ent);
    }
#else
  Entity *ent = newProp->castTo<Entity>();
  if(ent)
    {
    xAssert(database());
    ent->_database = database();
    }
#endif

  // is any prop in
  if(Property *prop = newProp->castTo<Property>())
    {
    bool parentComputed = isComputed() || _flags.hasFlag(ParentHasInput);
    if(input() || parentComputed)
      {
      prop->_flags.setFlag(ParentHasInput);
      }

    bool parentAffects = !isDynamic() && embeddedInstanceInformation()->affectsSiblings();
    if(output() || parentAffects || _flags.hasFlag(ParentHasOutput))
      {
      prop->_flags.setFlag(ParentHasOutput);
      }
    }
  xAssert(newProp->parent());
  }

void Container::internalRemove(Attribute *oldProp)
  {
  xAssert(oldProp);
  xAssert(oldProp->parent() == this);
  bool removed = false;

  DynamicPropertyInstanceInformation *oldPropInstInfo =
      const_cast<DynamicPropertyInstanceInformation*>(oldProp->dynamicBaseInstanceInformation());

  xsize oldIndex = oldProp->dynamicInstanceInformation()->index();
  DynamicPropertyInstanceInformation *indexUpdate = 0;

  if(oldProp == _dynamicChild)
    {
    xAssert(containedProperties() == 0);

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
    Attribute *prop = _dynamicChild;
    while(prop)
      {
      DynamicPropertyInstanceInformation *propInstInfo =
          const_cast<DynamicPropertyInstanceInformation*>(prop->dynamicBaseInstanceInformation());

      if(oldProp == propInstInfo->nextSibling())
        {
        xAssert((propIndex+1) >= containedProperties());

        removed = true;
        oldPropInstInfo->setInvalidIndex();

        auto next = oldPropInstInfo->nextSibling();
        propInstInfo->setNextSibling(next);

        if(next)
          {
          indexUpdate =
              const_cast<DynamicPropertyInstanceInformation*>(next->dynamicBaseInstanceInformation());
          }

        break;
        }
      propIndex++;
      prop = nextDynamicSibling(prop);
      }
    }

  xsize newIndex = oldIndex;
  while(indexUpdate)
    {
    indexUpdate->setIndex(newIndex);

    ++newIndex;

    if(!indexUpdate->nextSibling())
    {
      break;
    }

    indexUpdate =
        const_cast<DynamicPropertyInstanceInformation*>(
          indexUpdate->nextSibling()->dynamicBaseInstanceInformation());
    }

  internalUnsetup(oldProp);

  xAssert(removed);
  // not dynamic or has a parent
  xAssert(!oldPropInstInfo->isDynamic() || oldPropInstInfo->parent());
  oldPropInstInfo->setParent(0);
  oldPropInstInfo->setNextSibling(0);
  }

void Container::internalUnsetup(Attribute *oldProp)
  {
  if (Property *prop = oldProp->castTo<Property>())
    {
    Property::ConnectionChange::clearParentHasInputConnection(prop);
    Property::ConnectionChange::clearParentHasOutputConnection(prop);
    }
  }

const Attribute *Container::at(xsize i) const
  {
  return const_cast<Container*>(this)->at(i);
  }

Attribute *Container::at(xsize i)
  {
#if X_ASSERTS_ENABLED
  xsize idx = 0;
#endif

  xForeach(auto x, walker())
    {
#if X_ASSERTS_ENABLED
    xAssert(index(x) == idx);
    ++idx;
#endif

    if(!i)
      {
      return x;
      }
    --i;
    }

  return 0;
  }

Attribute *Container::nextDynamicSibling(const Attribute *p)
  {
  preGet();
  xAssert(p->isDynamic());
  return p->dynamicBaseInstanceInformation()->nextSibling();
  }

const Attribute *Container::nextDynamicSibling(const Attribute *p) const
  {
  preGet();
  xAssert(p->isDynamic());
  return p->dynamicBaseInstanceInformation()->nextSibling();
  }

Attribute *Container::firstChild()
  {
  const PropertyInformation* info = typeInformation();
  if(info->childCount())
    {
    const EmbeddedPropertyInstanceInformation *inst = info->childFromIndex(0);
    return inst->locate(this);
    }

  return firstDynamicChild();
  }

const Attribute *Container::firstChild() const
  {
  return ((Container*)this)->firstChild();
  }

Attribute *Container::lastChild()
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

const Attribute *Container::lastChild() const
  {
  return ((Container*)this)->lastChild();
  }


xsize Container::index(const Attribute* prop) const
  {
  SProfileFunction
  preGet();

  const PropertyInstanceInformation* bInfo = prop->baseInstanceInformation();
  if(bInfo->isDynamic())
    {
    return bInfo->dynamicInfo()->index();
    }

  return bInfo->embeddedInfo()->index();
  }

}
