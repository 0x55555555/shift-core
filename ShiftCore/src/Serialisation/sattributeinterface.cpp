#include "shift/Serialisation/sattributeinterface.h"
#include "shift/TypeInformation/spropertyinformation.h"


namespace Shift
{

//----------------------------------------------------------------------------------------------------------------------
// ChildBlock Impl
//----------------------------------------------------------------------------------------------------------------------
AttributeInterface::ChildBlock::ChildBlock(AttributeInterface::AttributeBlock *owner, AttributeInterface::ChildrenType type, Eks::AllocatorBase *alloc)
    : AttributeSubBlockHelper(owner),
      _activeChild(nullptr)
  {
  _owner->setChildren(this);

  _user = owner->saveData()->onBeginChildren(owner->user(), type, alloc);
  xAssert(_user);
  }

AttributeInterface::ChildBlock::~ChildBlock()
  {
  xAssert(!_activeChild);
  _owner->childrenComplete(this);
  }

Eks::UniquePointer<AttributeInterface::AttributeBlock> AttributeInterface::ChildBlock::addChild(
    const Name &name,
    const PropertyInformation *info,
    bool isDynamic,
    Eks::AllocatorBase *alloc)
  {
  xAssert(!_activeChild);

  auto out = alloc->createUnique<AttributeInterface::AttributeBlock>(this, name, info, isDynamic, alloc);
  xAssert(_activeChild);

  return std::move(out);
  }

void AttributeInterface::ChildBlock::setActiveChild(AttributeInterface::AttributeBlock *a)
  {
  xAssert(!_activeChild)
  _activeChild = a;
  }

void AttributeInterface::ChildBlock::childComplete(AttributeInterface::AttributeBlock *a)
  {
  xAssert(_activeChild);
  xAssert(a == _activeChild);

  _owner->saveData()->onChildComplete(user(), a->user());

  _activeChild = nullptr;
  }

//----------------------------------------------------------------------------------------------------------------------
// ValueBlock Impl
//----------------------------------------------------------------------------------------------------------------------
AttributeInterface::ValueBlock::ValueBlock(AttributeInterface::AttributeBlock *data, Eks::AllocatorBase *alloc)
    : AttributeSubBlockHelper(data)
  {
  _owner->setValues(this);

  _user = data->saveData()->onBeginValues(data->user(), alloc);
  xAssert(_user);
  }

AttributeInterface::ValueBlock::~ValueBlock()
  {
  _owner->valuesComplete(this);
  }

void AttributeInterface::ValueBlock::setValue(const Symbol &id, const SerialisationValue& value)
  {
  _owner->saveData()->onValue(user(), id, value);
  }

const SerialisationSymbol &AttributeInterface::ValueBlock::modeSymbol()
  {
  return _owner->saveData()->modeSymbol();
  }

const SerialisationSymbol &AttributeInterface::ValueBlock::inputSymbol()
  {
  return _owner->saveData()->inputSymbol();
  }

const SerialisationSymbol &AttributeInterface::ValueBlock::valueSymbol()
  {
  return _owner->saveData()->valueSymbol();
  }

//----------------------------------------------------------------------------------------------------------------------
// AttributeInterface::AttributeBlock Impl
//----------------------------------------------------------------------------------------------------------------------
AttributeInterface::AttributeBlock::AttributeBlock(
  ChildBlock *parent,
  const Name &name,
  const PropertyInformation *info,
  bool isDynamic,
  Eks::AllocatorBase *alloc)
    : _parent(parent),
      _data(parent ? parent->owner()->saveData() : nullptr),
      _isDynamic(isDynamic),
      _type(info),
      _values(nullptr),
      _children(nullptr),
      _hasValues(false),
      _hasChildren(false),
      _alloc(alloc)
  {
  init(name);
  }

AttributeInterface::AttributeBlock::~AttributeBlock()
  {
  if(_parent)
    {
    _parent->childComplete(this);
    }
  }

Eks::UniquePointer<AttributeInterface::ChildBlock> AttributeInterface::AttributeBlock::beginChildren(AttributeInterface::ChildrenType type, Eks::AllocatorBase *alloc)
  {
  xAssert(!_hasChildren);
  xAssert(!_children);

  auto out = alloc->createUnique<ChildBlock>(this, type, alloc);
  xAssert(_children);
  return out;
  }

void AttributeInterface::AttributeBlock::init(const Name &name)
  {
  if(_user)
    {
    return;
    }

  if (!_data)
    {
    return;
    }

  if(_parent)
    {
    _user = _data->onAddChild(_parent->user(), name, _alloc);

    if(_parent)
      {
      _parent->setActiveChild(this);
      }

    saveData()->addSavedType(_type, _isDynamic);
    }
  else
    {
    _user = _data->onAddChild(nullptr, name, _alloc);
    }
  }

void AttributeInterface::AttributeBlock::setRoot(AttributeInterface *data)
  {
  _data = data;
  init(Name());
  }

void AttributeInterface::AttributeBlock::setChildren(ChildBlock *vals)
  {
  xAssert(!_parent || _hasValues);
  xAssert(!_values);
  xAssert(!_children);
  _children = vals;
  }

void AttributeInterface::AttributeBlock::childrenComplete(ChildBlock *c)
  {
  xAssert(c == _children);
  xAssert(_children);

  saveData()->onChildrenComplete(user(), c->user());

  _hasChildren = true;
  _children = nullptr;
  }

Eks::UniquePointer<AttributeInterface::ValueBlock> AttributeInterface::AttributeBlock::beginValues(Eks::AllocatorBase *alloc)
  {
  xAssert(!_hasValues);
  xAssert(!_values);

  auto out = alloc->createUnique<ValueBlock>(this, alloc);

  if(_isDynamic)
    {
    xAssert(_type);
    TypedSerialisationValue<Name> str(&_type->typeName());
    out->setValue(saveData()->typeSymbol(), str);
    }

  xAssert(_values);
  return out;
  }

void AttributeInterface::AttributeBlock::setValues(ValueBlock *vals)
  {
  xAssert(!_children);
  xAssert(!_hasChildren);

  xAssert(!_values);
  _values = vals;
  }

void AttributeInterface::AttributeBlock::valuesComplete(ValueBlock *v)
  {
  xAssert(v == _values);
  xAssert(_values);

  saveData()->onValuesComplete(user(), v->user());

  _hasValues = true;
  _values = nullptr;
  }

AttributeInterface::RootBlock::RootBlock(bool includeRoot, AttributeInterface *ifc, const PropertyInformation *info, bool dynamic, Eks::AllocatorBase *alloc)
    : AttributeBlock(nullptr, Name(), info, dynamic, alloc)
  {
  setRoot(ifc);

  ifc->onBegin(user(), includeRoot, alloc);

  if(includeRoot)
    {
    ifc->addSavedType(info, dynamic);
    }
  }

AttributeInterface::RootBlock::~RootBlock()
  {
  saveData()->onEnd(user());
  }

Eks::UniquePointer<AttributeInterface::RootBlock> AttributeInterface::begin(bool includeRoot, const PropertyInformation *info, bool dynamic, Eks::AllocatorBase *alloc)
  {
  return alloc->createUnique<RootBlock>(includeRoot, this, info, dynamic, alloc);
  }

}
