#include "shift/Serialisation/sloader.h"
#include "shift/Properties/scontainer.h"
#include "shift/Properties/scontainer.inl"
#include "shift/Properties/sattribute.inl"
#include "shift/Properties/scontaineriterators.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/spropertytraits.h"

namespace Shift
{
class AttributeLoadHelper : public AttributeInterface::AttributeData
  {
public:
  AttributeLoadHelper(Container *par, Attribute *attr, const Eks::String &name, Eks::AllocatorBase *alloc)
      : parent(par),
        attribute(attr),
        name(name, alloc),
        allocator(alloc)
    {
    xAssert(alloc);
    }

  Container *parent;

  Attribute *attribute;
  Name name;
  Eks::AllocatorBase *allocator;
  };

class ChildLoadHelper : public AttributeInterface::ChildData
  {
public:
  ChildLoadHelper(Container *cont, Eks::AllocatorBase *alloc)
      : container(cont),
        editCache(cont->hasNamedChildren() ? cont->createEditCache(alloc) : nullptr)
    {
    }

  Container *container;
  Eks::UniquePointer<Container::EditCache> editCache;
  };

class ValueLoadHelper : public AttributeInterface::ValueData
  {
public:
  ValueLoadHelper(AttributeLoadHelper *attr)
      : owner(attr)
    {
    }

  AttributeLoadHelper *owner;
  };

class LoadBuilder::LoadData
  {
public:
  class LoadSymbol : public SerialisationSymbol, public SerialisationValue
    {
  public:
    void reset()
      {
      utf8Exists = false;
      utf8Value.clear();
      binaryExists = false;
      binaryValue.clear();
      }

    bool hasUtf8() const X_OVERRIDE
      {
      return utf8Exists;
      }

    bool hasBinary() const X_OVERRIDE
      {
      return binaryExists;
      }

    Eks::String asUtf8(Eks::AllocatorBase* a) const X_OVERRIDE
      {
      return Eks::String(utf8Value, a);
      }

    Eks::Vector<xuint8> asBinary(Eks::AllocatorBase* a) const X_OVERRIDE
      {
      return Eks::Vector<xuint8>(binaryValue, a);
      }


    mutable bool utf8Exists;
    mutable Eks::String utf8Value;
    mutable bool binaryExists;
    mutable Eks::Vector<xuint8> binaryValue;
    };

  LoadData(Eks::AllocatorBase *alloc)
      : allocator(alloc),
        resolveLater(alloc)
    {
    }

  void resetSymbols()
    {
    _mode.reset();
    _input.reset();
    _value.reset();
    _type.reset();
    }

  LoadSymbol _mode;
  LoadSymbol _input;
  LoadSymbol _value;
  LoadSymbol _type;
  Eks::AllocatorBase *allocator;
  Eks::UnorderedMap<Property *, Eks::String> resolveLater;
  };

LoadBuilder::LoadBlock::LoadBlock(LoadBuilder *w, Attribute *root, Eks::AllocatorBase *alloc)
    : IOBlock(w),
      _root(root),
      _loadedData(alloc)
  {
  }

LoadBuilder::LoadBuilder()
  {
  }

LoadBuilder::~LoadBuilder()
  {
  }

Eks::UniquePointer<LoadBuilder::LoadBlock> LoadBuilder::beginLoading(Attribute *root, Eks::AllocatorBase *alloc)
  {
  return Eks::Core::globalAllocator()->createUnique<LoadBlock>(this, root, alloc);
  }

void LoadBuilder::onBegin(AttributeData *root, bool includeRoot, Eks::AllocatorBase *alloc)
  {
  _currentData = alloc->createUnique<LoadData>(alloc);

  _currentData->allocator = alloc;

  auto data = root->as<AttributeLoadHelper>();
  auto loadBlock = static_cast<LoadBlock*>(activeBlock());

  if(includeRoot)
    {
    data->parent = loadBlock->root()->castTo<Container>();
    }
  else
    {
    data->attribute = loadBlock->root();
    data->parent = data->attribute->parent();
    }
  }

void LoadBuilder::onEnd(AttributeData *)
  {
  auto it = _currentData->resolveLater.begin();
  auto end = _currentData->resolveLater.end();
  for(; it != end; ++it)
    {
    auto& prop = it->first;
    auto& path = it->second;

    auto input = prop->resolvePath(path);

    xAssert(input);
    if(!input)
      {
      continue;
      }

    Property *inputProp = input->castTo<Property>();
    xAssert(inputProp);
    if(!inputProp)
      {
      continue;
      }

    prop->setInput(inputProp);
    }

  _currentData = nullptr;
  }

void LoadBuilder::addSavedType(const PropertyInformation *, bool)
  {
  }

const SerialisationSymbol &LoadBuilder::modeSymbol()
  {
  return _currentData->_mode;
  }

const SerialisationSymbol &LoadBuilder::inputSymbol()
  {
  return _currentData->_input;
  }

const SerialisationSymbol &LoadBuilder::valueSymbol()
  {
  return _currentData->_value;
  }

const SerialisationSymbol &LoadBuilder::typeSymbol()
  {
  return _currentData->_type;
  }

Eks::UniquePointer<AttributeInterface::ChildData> LoadBuilder::onBeginChildren(AttributeData *root, ChildrenType, Eks::AllocatorBase *alloc)
  {
  auto data = root->as<AttributeLoadHelper>();
  xAssert(data->attribute)

  auto container = data->attribute->castTo<Container>();
  xAssert(container);

  auto child = alloc->createUnique<ChildLoadHelper>(container, alloc);
  return std::move(child);
  }

void LoadBuilder::onChildrenComplete(AttributeData *, ChildData *)
  {
  }

Eks::UniquePointer<AttributeInterface::AttributeData> LoadBuilder::onAddChild(ChildData *data, const Name &name, Eks::AllocatorBase *alloc)
  {
  Container *parent = nullptr;
  Attribute *child = nullptr;
  if (data)
    {
    auto attr = data->as<ChildLoadHelper>();
    parent = attr->container;

    if(parent->hasNamedChildren())
      {
      child = parent->findChild(name);
      }
    // else, attribute is indexed, and that always means dynamic.
    }

  return alloc->createUnique<AttributeLoadHelper>(parent, child, name, alloc);
  }

void LoadBuilder::onChildComplete(ChildData *, AttributeData *)
  {
  }

Eks::UniquePointer<AttributeInterface::ValueData> LoadBuilder::onBeginValues(AttributeData *attr, Eks::AllocatorBase *alloc)
  {
  _currentData->resetSymbols();
  return alloc->createUnique<ValueLoadHelper>(attr->as<AttributeLoadHelper>());
  }

void LoadBuilder::onValue(ValueData *val, const Symbol &id, const SerialisationValue& value)
  {
  auto data = val->as<ValueLoadHelper>();

  const LoadData::LoadSymbol &sym = static_cast<const LoadData::LoadSymbol &>(id);

  if(value.hasBinary())
    {
    sym.binaryExists = true;
    sym.binaryValue = value.asBinary(data->owner->allocator);
    }
  else if(value.hasUtf8())
    {
    sym.utf8Exists = true;
    sym.utf8Value = value.asUtf8(data->owner->allocator);
    }
  }

void LoadBuilder::onValuesComplete(AttributeData *attr, ValueData *)
  {
  auto attrData = attr->as<AttributeLoadHelper>();

  struct LoadHelper : public AttributeLoader
    {
    const Symbol &modeSymbol() const X_OVERRIDE
      {
      return builder->modeSymbol();
      }
    const Symbol &inputSymbol() const X_OVERRIDE
      {
      return builder->inputSymbol();
      }
    const Symbol &valueSymbol() const X_OVERRIDE
      {
      return builder->valueSymbol();
      }

    Eks::AllocatorBase *temporaryAllocator() X_OVERRIDE
      {
      return tempAlloc;
      }

    Attribute *existingAttribute() X_OVERRIDE
      {
      return attributeWrapper->attribute;
      }

    const Name& name() X_OVERRIDE
      {
      xAssert(!existingAttribute());
      return attributeWrapper->name;
      }

    const PropertyInformation *type() X_OVERRIDE
      {
      xAssert(!existingAttribute());
      return typeInfo;
      }

    void resolveInputAfterLoad(Property *prop, const Eks::String &path) X_OVERRIDE
      {
      data->resolveLater[prop] = Eks::String(path, data->allocator);
      }

    const SerialisationValue& readValue(const Symbol &id) X_OVERRIDE
      {
      return static_cast<const LoadData::LoadSymbol &>(id);
      }

    Eks::AllocatorBase *tempAlloc;
    LoadBuilder *builder;
    LoadData *data;
    const PropertyInformation *typeInfo;
    AttributeLoadHelper *attributeWrapper;
    } helper;

  helper.tempAlloc = attrData->allocator;
  helper.builder = this;
  helper.data = _currentData.get();
  helper.attributeWrapper = attrData;

  const PropertyInformation *info = nullptr;
  Name type;
  if(helper.read(typeSymbol(), type))
    {
    info = TypeRegistry::findType(type);
    helper.typeInfo = info;
    }
  else if(attrData->attribute)
    {
    info = attrData->attribute->typeInformation();
    }
  else
    {
    xAssertFail();
    return;
    }
  xAssert(helper.typeInfo || attrData->attribute);

  attrData->attribute = info->functions().load(attrData->parent, helper);
  xAssert(attrData->attribute);

  auto block = static_cast<LoadBlock*>(activeBlock());
  if (attrData->parent == block->_root)
    {
    static_cast<LoadBlock*>(activeBlock())->_loadedData.pushBack(attrData->attribute);
    }
  }
}
