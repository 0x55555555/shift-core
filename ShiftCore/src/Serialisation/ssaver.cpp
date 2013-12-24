#include "shift/Serialisation/ssaver.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/Properties/sattribute.h"
#include "shift/Properties/scontainer.h"
#include "shift/Properties/scontaineriterators.h"

namespace Shift
{

class ChildBlock;
class ValueBlock;

//----------------------------------------------------------------------------------------------------------------------
// ChildBlock Def
//----------------------------------------------------------------------------------------------------------------------
class ChildBlock
  {
public:
  ChildBlock(Saver::AttributeBlock *owner, Saver::ChildrenType type, Eks::AllocatorBase *alloc);
  ~ChildBlock();

  Saver::AttributeBlock *owner() { return _owner; }

  /// \brief Begin writing an attribute, which ends when the destructor is called.
  Eks::UniquePointer<Saver::AttributeBlock> addChild(Attribute *a, Eks::AllocatorBase *alloc);

  Saver::ChildData *user() { return _user.value(); }

protected:
  /// \brief Begin writing an attribute, which ends when the destructor is called.
  void setActiveChild(Saver::AttributeBlock *a);

  /// \brief Called automatically when the Attribute Data above goes out of scope.
  void childComplete(Saver::AttributeBlock *);

private:
  Saver::AttributeBlock *_activeChild;
  Saver::AttributeBlock *_owner;

  Eks::UniquePointer<Saver::ChildData> _user;

  friend class Saver::AttributeBlock;
  };

//----------------------------------------------------------------------------------------------------------------------
// ValueBlock Def
//----------------------------------------------------------------------------------------------------------------------
class ValueBlock : public AttributeSaver
  {
public:
  ValueBlock(Saver::AttributeBlock *data, Eks::AllocatorBase *alloc);
  ~ValueBlock();

  /// \brief symbol for attribute mode.
  const Symbol &modeSymbol() X_OVERRIDE;
  /// \brief symbol for input mode.
  const Symbol &inputSymbol() X_OVERRIDE;
  /// \brief symbol for value mode.
  const Symbol &valueSymbol() X_OVERRIDE;
  /// \brief symbol for type mode.
  const Symbol &typeSymbol();

  void writeValue(const Symbol &id, const SerialisationValue& value) X_OVERRIDE;

  Saver::AttributeBlock *owner() { return _owner; }

  Saver::ValueData *user() { return _user.value(); }

private:
  Saver::AttributeBlock *_owner;

  Eks::UniquePointer<Saver::ValueData> _user;
  };

//----------------------------------------------------------------------------------------------------------------------
// AttributeBlock Def
//----------------------------------------------------------------------------------------------------------------------
class Saver::AttributeBlock
  {
public:
  AttributeBlock(ChildBlock *parent, Attribute *attr, Eks::AllocatorBase *alloc);
  ~AttributeBlock();

  /// \brief get the owning save.
  Saver* saveData() { return _data; }
  /// \brief Get the attribute being saved.
  Attribute *attribute() const { return _attribute; }

  /// \brief Begin a writing child attributes.
  Eks::UniquePointer<ChildBlock> beginChildren(Saver::ChildrenType type, Eks::AllocatorBase *alloc);

  /// \brief Begin a block of values.
  Eks::UniquePointer<ValueBlock> beginValues(Eks::AllocatorBase *alloc);

  void setRoot(Saver *data);
  bool isRoot() const { return _parent == nullptr; }

  Saver::AttributeData *user() { return _user.value(); }

protected:
  void setValues(ValueBlock *vals);
  void setChildren(ChildBlock *vals);

  void childrenComplete(ChildBlock *);
  void valuesComplete(ValueBlock *);

private:
  void initUser();
  Attribute *_attribute;
  ChildBlock *_parent;
  Saver *_data;

  ValueBlock *_values;
  ChildBlock *_children;

  Eks::UniquePointer<Saver::AttributeData> _user;

  bool _hasValues;
  bool _hasChildren;

  Eks::AllocatorBase *_alloc;

  friend class Saver;
  friend class ChildBlock;
  friend class ValueBlock;
  };

//----------------------------------------------------------------------------------------------------------------------
// ChildBlock Impl
//----------------------------------------------------------------------------------------------------------------------
ChildBlock::ChildBlock(Saver::AttributeBlock *owner, Saver::ChildrenType type, Eks::AllocatorBase *alloc)
    : _activeChild(nullptr),
      _owner(owner)
  {
  _owner->setChildren(this);

  _user = owner->saveData()->onBeginChildren(owner->user(), type, alloc);
  xAssert(_user);
  }

ChildBlock::~ChildBlock()
  {
  xAssert(!_activeChild);
  _owner->childrenComplete(this);
  }

Eks::UniquePointer<Saver::AttributeBlock> ChildBlock::addChild(Attribute *a, Eks::AllocatorBase *alloc)
  {
  xAssert(!_activeChild);

  auto out = alloc->createUnique<Saver::AttributeBlock>(this, a, alloc);
  xAssert(_activeChild);

  return std::move(out);
  }

void ChildBlock::setActiveChild(Saver::AttributeBlock *a)
  {
  xAssert(!_activeChild)
  _activeChild = a;
  }

void ChildBlock::childComplete(Saver::AttributeBlock *a)
  {
  xAssert(_activeChild);
  xAssert(a == _activeChild);

  _owner->saveData()->onChildComplete(user(), a->user());

  _activeChild = nullptr;
  }

//----------------------------------------------------------------------------------------------------------------------
// ValueBlock Impl
//----------------------------------------------------------------------------------------------------------------------
ValueBlock::ValueBlock(Saver::AttributeBlock *data, Eks::AllocatorBase *alloc) : _owner(data)
  {
  _owner->setValues(this);

  _user = data->saveData()->onBeginValues(data->user(), alloc);
  xAssert(_user);
  }

ValueBlock::~ValueBlock()
  {
  _owner->valuesComplete(this);
  }

const SerialisationSymbol &ValueBlock::modeSymbol()
  {
  return _owner->saveData()->modeSymbol();
  }

const SerialisationSymbol &ValueBlock::inputSymbol()
  {
  return _owner->saveData()->inputSymbol();
  }

const SerialisationSymbol &ValueBlock::valueSymbol()
  {
  return _owner->saveData()->valueSymbol();
  }

const SerialisationSymbol &ValueBlock::typeSymbol()
  {
  return _owner->saveData()->typeSymbol();
  }

void ValueBlock::writeValue(const Symbol &id, const SerialisationValue& value)
  {
  _owner->saveData()->onWriteValue(user(), id, value);
  }

//----------------------------------------------------------------------------------------------------------------------
// Saver::AttributeBlock Impl
//----------------------------------------------------------------------------------------------------------------------
Saver::AttributeBlock::AttributeBlock(ChildBlock *parent, Attribute *attr, Eks::AllocatorBase *alloc)
    : _attribute(attr),
      _parent(parent),
      _data(parent ? parent->owner()->saveData() : nullptr),
      _values(nullptr),
      _children(nullptr),
      _hasValues(false),
      _hasChildren(false),
      _alloc(alloc)
  {
  initUser();
  }

Saver::AttributeBlock::~AttributeBlock()
  {
  if(_parent)
    {
    _parent->childComplete(this);
    }
  }

Eks::UniquePointer<ChildBlock> Saver::AttributeBlock::beginChildren(Saver::ChildrenType type, Eks::AllocatorBase *alloc)
  {
  xAssert(!_hasChildren);
  xAssert(!_children);
  xAssert(attribute()->castTo<Container>());

  auto out = alloc->createUnique<ChildBlock>(this, type, alloc);
  xAssert(_children);
  return out;
  }

void Saver::AttributeBlock::initUser()
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
    _user = _data->onAddChild(_parent->user(), _attribute, _alloc);

    if(_parent)
      {
      _parent->setActiveChild(this);
      }
    }
  else
    {
    _user = _data->onAddChild(nullptr, _attribute, _alloc);
    }
  }

void Saver::AttributeBlock::setRoot(Saver *data)
  {
  _data = data;
  initUser();
  }

void Saver::AttributeBlock::setChildren(ChildBlock *vals)
  {
  xAssert(!_children);
  _children = vals;
  }

void Saver::AttributeBlock::childrenComplete(ChildBlock *c)
  {
  xAssert(c == _children);
  xAssert(_children);

  saveData()->onChildrenComplete(user(), c->user());

  _hasChildren = true;
  _children = nullptr;
  }

Eks::UniquePointer<ValueBlock> Saver::AttributeBlock::beginValues(Eks::AllocatorBase *alloc)
  {
  xAssert(!_hasValues);
  xAssert(!_values);

  auto out = alloc->createUnique<ValueBlock>(this, alloc);
  xAssert(_values);
  return out;
  }

void Saver::AttributeBlock::setValues(ValueBlock *vals)
  {
  xAssert(!_values);
  _values = vals;
  }

void Saver::AttributeBlock::valuesComplete(ValueBlock *v)
  {
  xAssert(v == _values);
  xAssert(_values);

  saveData()->onValuesComplete(user(), v->user());

  _hasValues = true;
  _values = nullptr;
  }

//----------------------------------------------------------------------------------------------------------------------
// Saver::WriteBlock Impl
//----------------------------------------------------------------------------------------------------------------------
Saver::WriteBlock::WriteBlock(Saver* w, QIODevice *device)
  : _writer(w),
    _device(device),
    _writing(false),
    _written(false)
  {
  xAssert(!w->_block);
  w->_block = this;
  }

Saver::WriteBlock::~WriteBlock()
  {
  _writer->_block = nullptr;
  }

//----------------------------------------------------------------------------------------------------------------------
// Saver::SaveData Impl
//----------------------------------------------------------------------------------------------------------------------
Saver::Saver()
    : _block(nullptr)
  {
  }

Saver::WriteBlock Saver::beginWriting(QIODevice *device)
  {
  return WriteBlock(this, device);
  }

void Saver::beginSave(Attribute *root, Eks::AllocatorBase *alloc)
  {
  xAssert(_block);
  xAssert(!_block->_writing);
  xAssert(!_block->_written);

  _rootAttribute = root;

  _block->_writing = true;

  _rootBlock = alloc->createUnique<AttributeBlock>(nullptr, root, alloc);
  _rootBlock->setRoot(this);

  onBeginSave(root, alloc);
  }

void Saver::endSave()
  {
  onEndSave();
  _rootBlock = nullptr;

  xAssert(_block);
  xAssert(_block->_writing);
  xAssert(!_block->_written);

  _block->_writing = false;
  _block->_written = true;
  }

Saver::AttributeData *Saver::rootData()
  {
  return _rootBlock->user();
  }


void SaveBuilder::save(Attribute *attr, bool includeRoot, Saver *receiver)
  {
  Eks::TemporaryAllocator alloc(attr->temporaryAllocator());

  receiver->beginSave(attr, &alloc);

  receiver->setIncludeRoot(includeRoot);

  visitAttribute(receiver->rootBlock(), &alloc);

  receiver->endSave();
  }

void SaveBuilder::visitAttribute(Saver::AttributeBlock *data, Eks::AllocatorBase *alloc)
  {
  visitValues(data, alloc);

  visitChildren(data, alloc);
  }

void SaveBuilder::visitValues(Saver::AttributeBlock *data, Eks::AllocatorBase *alloc)
  {
  auto attr = data->attribute();
  xAssert(attr);

  const PropertyInformation *info = attr->typeInformation();
  xAssert(info);

  const bool dyn = attr->isDynamic();

  data->saveData()->addSavedType(info, dyn);

  auto dataAttrs = data->beginValues(alloc);
  if(dyn)
    {
    dataAttrs->write(dataAttrs->typeSymbol(), info->typeName());
    }

  info->functions().save(attr, *dataAttrs.value());
  }

void SaveBuilder::visitChildren(Saver::AttributeBlock *attr, Eks::AllocatorBase *attrAlloc)
  {
  Container* cont = attr->attribute()->castTo<Container>();
  if (!cont)
    {
    return;
    }

  bool shouldSaveAnyChildren = false;
  xForeach(auto child, cont->walker())
    {
    const PropertyInformation *info = child->typeInformation();

    if(info->functions().shouldSave(child))
      {
      shouldSaveAnyChildren = true;
      break;
      }
    }

  if(shouldSaveAnyChildren)
    {
    auto childrenType = cont->hasNamedChildren() ? Saver::Named : Saver::Indexed;

    auto children = attr->beginChildren(childrenType, attrAlloc);
    xAssert(children);

    xForeach(auto child, cont->walker())
      {
      const PropertyInformation *info = child->typeInformation();

      if(info->functions().shouldSave(child))
        {
        Eks::TemporaryAllocator alloc(child->temporaryAllocator());

        auto attr = children->addChild(child, &alloc);

        if(info->functions().shouldSaveValue(child))
          {
          visitAttribute(attr.value(), &alloc);
          }
        }
      }
    }
  }

}
