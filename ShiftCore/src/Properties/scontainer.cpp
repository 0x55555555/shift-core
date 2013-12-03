#include "shift/Properties/scontainer.h"
#include "shift/Properties/scontainer.inl"
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

Container::Container()
  : _dynamicChild(nullptr),
    _lastDynamicChild(nullptr)
  {
  }

Container::~Container()
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
    PropertyDoChange(ContainerTreeChange, this, c, p, X_UINT8_SENTINEL);
    }
  else
    {
    PropertyDoChange(ContainerTreeChange, this, c, p, X_UINT8_SENTINEL);
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
  PropertyDoChange(ContainerTreeChange, this, (Container*)0, oldProp, index(oldProp));
  }

const Attribute *Container::at(xsize i) const
  {
  return const_cast<Container*>(this)->at(i);
  }

Attribute *Container::at(xsize i)
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
  if(_lastDynamicChild)
    {
    return _lastDynamicChild;
    }

  const PropertyInformation* info = typeInformation();
  xsize s = info->childCount();
  if(s == 0)
    {
    return nullptr;
    }

  return info->childFromIndex(s-1)->locate(this);
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
    xsize idx = containedProperties();
    auto child = _dynamicChild;
    while(child && child != prop)
      {
      ++idx;
      child = child->dynamicBaseInstanceInformation()->nextSibling();
      }

    return idx;
    }

  return bInfo->embeddedInfo()->index();
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

class Container::EditCache
  {
XProperties:
  XROProperty(const Container *, container)
  XROProperty(Eks::AllocatorBase *, allocator)

public:
  EditCache(const Container *c, Eks::AllocatorBase *a)
      : _container(c), _allocator(a)
    {
    }
  };

Eks::UniquePointer<Container::EditCache> Container::createEditCache(Eks::AllocatorBase *alloc) const
  {
  return alloc->createUnique<EditCache>(this, alloc);
  }

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

  if(index != X_SIZE_SENTINEL)
    {
    xsize dynamicIndex = index - containedProperties();
    Attribute *justBefore = ChildLL::insert(&_dynamicChild, newProp, dynamicIndex);

    if (!justBefore && dynamicIndex != 0)
      {
      xAssertFail();
      return;
      }
    }
  else
    {
    if(_lastDynamicChild)
      {
      ChildLL::appendAt(ChildLL::getNextLocation(_lastDynamicChild), newProp);
      }
    else
      {
      xsize insertedIndex = 0;
      ChildLL::append(&_dynamicChild, newProp, &insertedIndex);
      }
    }

  if(!ChildLL::getNext(newProp))
    {
    _lastDynamicChild = newProp;
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

  Attribute* beforeRemoved = ChildLL::remove(&_dynamicChild, oldProp);

  if(_lastDynamicChild == oldProp)
    {
    _lastDynamicChild = nullptr;

    if (beforeRemoved)
      {
      auto child = beforeRemoved;
      while(child)
        {
        _lastDynamicChild = child;
        child = ChildLL::getNext(child);
        }
      }
    }

  internalUnsetup(oldProp);

  // not dynamic or has a parent
  auto oldPropInstInfo = ChildLL::getInstanceInfo(oldProp);
  xAssert(!oldPropInstInfo->isDynamic() || oldPropInstInfo->parent());
  oldPropInstInfo->setParent(nullptr);
  xAssert(!oldPropInstInfo->nextSibling());
  }

void Container::internalUnsetup(Attribute *oldProp)
  {
  if (Property *prop = oldProp->castTo<Property>())
    {
    Property::ConnectionChange::clearParentHasInputConnection(prop);
    Property::ConnectionChange::clearParentHasOutputConnection(prop);
    }
  }

}
