#include "shift/Serialisation/sjsonio.h"
#include "shift/sentity.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/Changes/shandler.h"
#include "QDebug"
#include "../Serialisation/JsonParser/JSON_parser.h"
#include "Utilities/XEventLogger.h"
#include "Utilities/XJsonWriter.h"

static const char *formatVersion = "2";

namespace Shift
{

#define VERSION_KEY "version"
#define NO_ROOT_KEY "noroot"
#define NO_ROOT_ON "1"
#define NO_ROOT_OFF "0"
#define TYPES_KEY "types"
#define DATA_KEY "data"
#define DYNAMIC_COUNT_KEY "dynamicCount"
#define CHILDREN_KEY "contents"
#define INPUT_KEY "input"
#define MODE_KEY "mode"
#define VALUE_KEY "value"
#define TYPE_KEY "type"

//----------------------------------------------------------------------------------------------------------------------
// String Symbol Def
//----------------------------------------------------------------------------------------------------------------------
class StringSymbol : public SerialisationSymbol
  {
public:
  StringSymbol(const char* data) : str(data)
    {
    }

  const char* str;
  };

//----------------------------------------------------------------------------------------------------------------------
// JSONSaver::JSONChildSaver Def
//----------------------------------------------------------------------------------------------------------------------
class JSONSaver::JSONChildSaver : public Saver::ChildData
  {
public:
  JSONChildSaver(Saver::ChildrenType t)
      : _childType(t)
    {
    }

  bool isNamed() const { return _childType == Named; }

private:
  Saver::ChildrenType _childType;
  };

//----------------------------------------------------------------------------------------------------------------------
// JSONSaver::JSONValueSaver Def
//----------------------------------------------------------------------------------------------------------------------
class JSONSaver::JSONValueSaver : public Saver::ValueData
  {
public:
  JSONValueSaver(Eks::AllocatorBase *alloc, Eks::JSONWriter *writer)
      : allocator(alloc),
        allValuesWriter(alloc, writer),
        attrCount(0),
        hasValueSymbolBeenWritten(false),
        valueOnly(alloc)
    {
    allValuesWriter.tabIn();
    }

  bool hasValues() { return attrCount > 0; }
  bool hasOnlyWrittenValueSymbol() { return attrCount == 1 && hasValueSymbolBeenWritten; }

  const Eks::String& valueSymbolDataOnly() { return valueOnly; }
  const Eks::String& allValues() { return allValuesWriter.string(); }

  Eks::AllocatorBase *allocator;

  Eks::JSONWriter allValuesWriter;

  xsize attrCount;
  bool hasValueSymbolBeenWritten;

  Eks::String valueOnly;
  };

//----------------------------------------------------------------------------------------------------------------------
// JSONSaver::JSONAttributeSaver Def
//----------------------------------------------------------------------------------------------------------------------
class JSONSaver::JSONAttributeSaver : public Saver::AttributeData
  {
XProperties:
  XROProperty(bool, includeRoot);

public:
  JSONAttributeSaver()
      : _includeRoot(true),
        hasBegunObject(false)
    {
    }

  void setIncludeRoot(bool include)
    {
    xAssert(!hasBegunObject);

    _includeRoot = include;
    }

  bool hasBegunObject;
  };

//----------------------------------------------------------------------------------------------------------------------
// JSONSaver::Impl Def
//----------------------------------------------------------------------------------------------------------------------
class JSONSaver::CurrentSaveData
  {
public:
  CurrentSaveData(Eks::AllocatorBase *alloc)
    : _mode(MODE_KEY),
      _input(INPUT_KEY),
      _value(VALUE_KEY),
      _type(TYPE_KEY),
      _children(CHILDREN_KEY),
      _writer(alloc),
      _typeMap(InfoSorter(), alloc),
      _allocator(alloc)
    {
    }

  struct TypeData
    {
    TypeData() : totalCount(0), dynamicCount(0) { }
    xsize totalCount;
    xsize dynamicCount;
    };

  StringSymbol _mode, _input, _value, _type, _children;
  Eks::JSONWriter _writer;

  struct InfoSorter
    {
    bool operator() (const PropertyInformation *lhs, const PropertyInformation *rhs) const
      {
      return lhs->typeName() < rhs->typeName();
      }
    };

  std::map<const PropertyInformation *, TypeData, InfoSorter, Eks::TypedAllocator<std::pair<const PropertyInformation *, TypeData> > > _typeMap;
  Eks::AllocatorBase *_allocator;
  };

//----------------------------------------------------------------------------------------------------------------------
// JSONSaver Impl
//----------------------------------------------------------------------------------------------------------------------
JSONSaver::JSONSaver() : _autoWhitespace(false)
  {
  }

void JSONSaver::onBegin(AttributeData *root, bool includeRoot, Eks::AllocatorBase *alloc)
  {
  Saver::onBegin(root, includeRoot, alloc);
  _data = alloc->createUnique<CurrentSaveData>(alloc);

  _data->_writer.setNiceFormatting(autoWhitespace());
  _data->_writer.tabIn();
  _data->_allocator = alloc;

  root->as<JSONAttributeSaver>()->setIncludeRoot(includeRoot);
  }

void JSONSaver::onEnd(AttributeData *root)
  {
  completeAttribute(root->as<JSONAttributeSaver>());
  emitJson(_data->_allocator, root, activeDevice());

  _data = nullptr;

  Saver::onEnd(root);
  }

void JSONSaver::emitJson(Eks::AllocatorBase *allocator, AttributeData *rootData, QIODevice *dev)
  {
  Eks::JSONWriter writer(allocator);
  writer.setNiceFormatting(_data->_writer.niceFormatting());

  writer.beginObject();

  writer.beginObjectElement(VERSION_KEY);
  writer.addValueForElement(formatVersion);
  writer.endElement();

  writer.beginObjectElement(TYPES_KEY);
  writer.beginObject();

  Eks::String tempString(allocator);

  auto it = _data->_typeMap.begin();
  auto end = _data->_typeMap.end();
  for(; it != end; ++it)
    {
    auto type = it->first;
    const auto& data = it->second;

    writer.beginObjectElement(type->typeName().data());

    writer.beginObject();

    tempString.clear();
    tempString.appendType(data.dynamicCount);

    writer.beginObjectElement(DYNAMIC_COUNT_KEY);
    writer.addValueForElement(tempString.data());
    writer.endElement();

    writer.end();

    writer.endElement();
    }

  writer.end();
  writer.endElement();

  auto root = rootData->as<JSONAttributeSaver>();

  writer.beginObjectElement(NO_ROOT_KEY);
  writer.addValueForElement(root->includeRoot() ? NO_ROOT_OFF : NO_ROOT_ON);
  writer.endElement();

  writer.beginObjectElement(DATA_KEY);

  dev->write(writer.string().data(), writer.string().length());
  dev->write(_data->_writer.string().data());
  if(_data->_writer.niceFormatting())
    {
    dev->write("\n}\n");
    }
  else
    {
    dev->write("}");
    }

  writer.endElement();
  writer.end();
  }

void JSONSaver::addSavedType(const PropertyInformation *info, bool dynamic)
  {
  CurrentSaveData::TypeData &data = _data->_typeMap[info];

  ++data.totalCount;

  if(dynamic)
    {
    ++data.dynamicCount;
    }
  }

const SerialisationSymbol &JSONSaver::modeSymbol()
  {
  return _data->_mode;
  }

const SerialisationSymbol &JSONSaver::inputSymbol()
  {
  return _data->_input;
  }

const SerialisationSymbol &JSONSaver::valueSymbol()
  {
  return _data->_value;
  }

const SerialisationSymbol &JSONSaver::typeSymbol()
  {
  return _data->_type;
  }

const SerialisationSymbol &JSONSaver::childrenSymbol()
  {
  return _data->_children;
  }

Eks::UniquePointer<Saver::ChildData> JSONSaver::onBeginChildren(AttributeData *data, Saver::ChildrenType type, Eks::AllocatorBase *alloc)
  {
  auto attr = data->as<JSONSaver::JSONAttributeSaver>();
  // if this isnt true, the maybe value was already written on its own?
  // this shouldnt happen for things with children - they dont have values...
  xAssert(attr->hasBegunObject || !attr->includeRoot());

  if(attr->includeRoot())
    {
    const Symbol& sym = childrenSymbol();
    _data->_writer.beginObjectElement(static_cast<const StringSymbol&>(sym).str);
    }

  auto a = alloc->createUnique<JSONSaver::JSONChildSaver>(type);

  if(a->isNamed())
    {
    _data->_writer.beginObject();
    }
  else
    {
    _data->_writer.beginArray();
    }

  return std::move(a);
  }

void JSONSaver::onChildrenComplete(AttributeData *data, ChildData *)
  {
  auto attr = data->as<JSONSaver::JSONAttributeSaver>();

  _data->_writer.end();

  if(attr->includeRoot())
    {
    _data->_writer.endElement();
    }
  }

Eks::UniquePointer<Saver::AttributeData> JSONSaver::onAddChild(Saver::ChildData *parent, const Shift::Name &name, Eks::AllocatorBase *alloc)
  {
  if(parent)
    {
    if(parent->as<JSONSaver::JSONChildSaver>()->isNamed())
      {
      _data->_writer.beginObjectElement(name.data());
      }
    else
      {
      _data->_writer.beginArrayElement();
      }
    }

  return alloc->createUnique<JSONSaver::JSONAttributeSaver>();
  }

void JSONSaver::onChildComplete(Saver::ChildData *, AttributeData *child)
  {
  completeAttribute(child);
  _data->_writer.endElement();
  }

void JSONSaver::completeAttribute(AttributeData *data)
  {
  if(data->as<JSONSaver::JSONAttributeSaver>()->hasBegunObject)
    {
    _data->_writer.end();
    }
  }

Eks::UniquePointer<Saver::ValueData> JSONSaver::onBeginValues(AttributeData *, Eks::AllocatorBase *alloc)
  {
  return alloc->createUnique<JSONSaver::JSONValueSaver>(alloc, &_data->_writer);
  }

void JSONSaver::onValuesComplete(AttributeData *data, ValueData *v)
  {
  auto values = v->as<JSONValueSaver>();
  auto attr = data->as<JSONSaver::JSONAttributeSaver>();

  if(attr->includeRoot())
    {
    if(values->hasOnlyWrittenValueSymbol())
      {
      _data->_writer.addValueForElement(values->valueSymbolDataOnly().data());
      }
    else
      {
      _data->_writer.beginObject();
      attr->hasBegunObject = true;

      if(values->hasValues())
        {
        _data->_writer.addBlock(values->allValues().data(), true);
        }
      }
    }
  }

void JSONSaver::onValue(Saver::ValueData *v, const Symbol &id, const SerialisationValue& value)
  {
  auto values = static_cast<JSONValueSaver*>(v);
  Eks::String valueStr = value.asUtf8(values->allocator);

  if (&id == &valueSymbol())
    {
    xAssert(!values->hasValueSymbolBeenWritten);
    values->hasValueSymbolBeenWritten = true;

    values->valueOnly = valueStr;
    }

  auto &sym = static_cast<const StringSymbol &>(id);

  const char *data = valueStr.data() ? valueStr.data() : "";

  values->allValuesWriter.addKeyValueStandalone(sym.str, data, values->attrCount != 0);

  xAssert(values->allValuesWriter.string().data());
  ++values->attrCount;
  }


//----------------------------------------------------------------------------------------------------------------------
// JSONLoaderImpl Impl
//----------------------------------------------------------------------------------------------------------------------

class JSONLoaderImpl
  {
public:
  JSONLoaderImpl(Eks::AllocatorBase *alloc, AttributeInterface *ifc)
    : _current(Start),
      _loadStack(alloc),
      _specialLoading(None),
      _currentKey(nullptr),
      _noRoot(false),
      _alloc(alloc),
      _interface(ifc)
    {
    JSON_config config;

    init_JSON_config(&config);

    config.depth = 19;
    config.callback = &callback;
    config.callback_ctx = this;
    config.allow_comments = 1;
    config.handle_floats_manually = 0;

    _jc = new_JSON_parser(&config);
    }

  ~JSONLoaderImpl()
    {
    delete_JSON_parser(_jc);
    }

  JSON_parser_struct* _jc;
  mutable QIODevice *_device;

  static int callback(void *ctx, int type, const JSON_value_struct* value);

  enum State
    {
    Start,
    Header,
    HeaderAwaitingValue,
    ContainedTypes,
    ContainedTypesAwaitingValue,
    ContainedType,
    ContainedTypeAwaitingValue,
    Attribute,
    AttributeAwaitingValue,
    AttributeAwaitingChildren,
    IndexedChildren,
    NamedChildren,
    NamedChildKey,
    MaxState
    } _current;

  Eks::UniquePointer<AttributeInterface::RootBlock> _rootBlock;

  struct StateData
    {
    StateData()
      {
      }
    StateData(StateData &&oth)
      : attribute(std::move(oth.attribute)),
        values(std::move(oth.values)),
        children(std::move(oth.children)),
        namedChildren(oth.namedChildren)
      {
      }
    StateData &operator=(StateData &&oth)
      {
      attribute = std::move(oth.attribute);
      values = std::move(oth.values);
      children = std::move(oth.children);
      namedChildren = oth.namedChildren;
      return *this;
      }

    Eks::UniquePointer<AttributeInterface::AttributeBlock> attribute;
    Eks::UniquePointer<AttributeInterface::ValueBlock> values;
    Eks::UniquePointer<AttributeInterface::ChildBlock> children;
    bool namedChildren;

  private:
    X_DISABLE_COPY(StateData);
    };

  Eks::Vector<StateData> _loadStack;

  enum SpecialSymbols
    {
    NoRoot,
    Version,
    AttributeKey,
    ContainedTypesKey,
    Type,
    None
    } _specialLoading;
  const AttributeInterface::Symbol *_currentKey;
  Name _loadedName;

  bool _noRoot;

  Eks::AllocatorBase *_alloc;

  AttributeInterface *_interface;
  };


typedef bool (*ActionFunction)(JSONLoaderImpl *loader, int type, const JSON_value *value);

bool err(JSONLoaderImpl *, int, const JSON_value *)
  {
  xAssertFail();
  return false;
  }

template <JSONLoaderImpl::State S> bool to(JSONLoaderImpl *loader, int, const JSON_value *)
  {
  loader->_current = S;
  return true;
  }

void initAttribute(JSONLoaderImpl *loader, const PropertyInformation *infoHint)
  {
  auto &back = loader->_loadStack.back();
  if(!back.attribute)
    {
    if(loader->_loadStack.size() == 1)
      {
      back.attribute = loader->_interface->begin(!loader->_noRoot, infoHint, infoHint != nullptr, loader->_alloc);

      if(!loader->_noRoot)
        {
        back.values = back.attribute->beginValues(loader->_alloc);
        }
      }
    else
      {
      auto &last = loader->_loadStack[loader->_loadStack.size() - 2];
      back.attribute = last.children->addChild(loader->_loadedName, infoHint, infoHint != nullptr, loader->_alloc);
      back.values = back.attribute->beginValues(loader->_alloc);
      }

    loader->_loadedName.clear();
    }
  }

bool children(JSONLoaderImpl *loader, int type, const JSON_value *)
  {
  auto &back = loader->_loadStack.back();

  initAttribute(loader, nullptr);

  back.values = nullptr;

  bool named = type == JSON_T_OBJECT_BEGIN;
  if (named)
    {
    back.children = back.attribute->beginChildren(AttributeInterface::Named, loader->_alloc);
    back.namedChildren = true;

    loader->_current = JSONLoaderImpl::NamedChildren;
    }
  else
    {
    back.children = back.attribute->beginChildren(AttributeInterface::Indexed, loader->_alloc);
    back.namedChildren = false;

    loader->_current = JSONLoaderImpl::IndexedChildren;
    }

  return true;
  }

bool endChildren(JSONLoaderImpl *loader, int, const JSON_value *)
  {
  auto &back = loader->_loadStack.back();
  xAssert(back.children);
  back.children = nullptr;
  loader->_current = JSONLoaderImpl::Attribute;
  return true;
  }

bool childName(JSONLoaderImpl *loader, int, const JSON_value *val)
  {
  loader->_loadedName = val->vu.str.value;
  loader->_current = JSONLoaderImpl::NamedChildKey;
  return true;
  }

bool addChild(JSONLoaderImpl *loader, int, const JSON_value *)
  {
  loader->_loadStack.createBack();
  loader->_current = JSONLoaderImpl::Attribute;
  return true;
  }

bool endChild(JSONLoaderImpl *loader, int, const JSON_value *)
  {
  loader->_loadStack.popBack();

  if (loader->_loadStack.size())
    {
    auto &back = loader->_loadStack.back();
    xAssert(back.children);

    if(back.namedChildren)
      {
      loader->_current = JSONLoaderImpl::NamedChildren;
      }
    else
      {
      loader->_current = JSONLoaderImpl::IndexedChildren;
      }
    }
  else
    {
    loader->_current = JSONLoaderImpl::Header;
    }
  return true;
  }

bool attrKey(JSONLoaderImpl *loader, int, const JSON_value *val)
  {
  loader->_currentKey = nullptr;
  loader->_specialLoading = JSONLoaderImpl::None;

  auto &back = loader->_loadStack.back();

  if(strcmp(val->vu.str.value, TYPE_KEY) == 0)
    {
    loader->_specialLoading = JSONLoaderImpl::Type;
    }
  else if(strcmp(val->vu.str.value, MODE_KEY) == 0)
    {
    xAssert(back.values);
    loader->_currentKey = &back.values->modeSymbol();
    }
  else if(strcmp(val->vu.str.value, INPUT_KEY) == 0)
    {
    xAssert(back.values);
    loader->_currentKey = &back.values->inputSymbol();
    }
  else if(strcmp(val->vu.str.value, VALUE_KEY) == 0)
    {
    xAssert(back.values);
    loader->_currentKey = &back.values->valueSymbol();
    }
  else if(strcmp(val->vu.str.value, CHILDREN_KEY) == 0)
    {
    loader->_current = JSONLoaderImpl::AttributeAwaitingChildren;
    return true;
    }
  else
    {
    return false;
    }

  loader->_current = JSONLoaderImpl::AttributeAwaitingValue;
  return true;
  }

bool attrVal(JSONLoaderImpl *loader, int, const JSON_value *val)
  {
  auto &back = loader->_loadStack.back();

  const PropertyInformation *info = nullptr;
  if(loader->_specialLoading == JSONLoaderImpl::Type)
    {
    info = TypeRegistry::findType(val->vu.str.value);
    }

  if(!back.attribute)
    {
    initAttribute(loader, info);
    }

  if(loader->_specialLoading == JSONLoaderImpl::None)
    {
    xAssert(loader->_currentKey);

    struct SerialisationValueWrapper : public SerialisationValue
      {
      bool hasUtf8() const X_OVERRIDE { return true; }
      bool hasBinary() const X_OVERRIDE { return false; }

      Eks::String asUtf8(Eks::AllocatorBase* a) const X_OVERRIDE
        {
        return Eks::String(data, a);
        }

      const char* data;
      } wrap;

    wrap.data = val->vu.str.value;

    back.values->setValue(*loader->_currentKey, wrap);
    }

  loader->_specialLoading = JSONLoaderImpl::None;
  loader->_currentKey = nullptr;
  loader->_current = JSONLoaderImpl::Attribute;
  return true;
  }

bool addSimpleChild(JSONLoaderImpl *loader, int type, const JSON_value *val)
  {
  addChild(loader, type, val);

  initAttribute(loader, nullptr);

  auto &back = loader->_loadStack.back();

  xAssert(back.values);
  loader->_currentKey = &back.values->valueSymbol();
  loader->_current = JSONLoaderImpl::AttributeAwaitingValue;

  attrVal(loader, type, val);

  endChild(loader, type, val);

  return true;
  }

bool headerKey(JSONLoaderImpl *loader, int, const JSON_value *val)
  {
  if(strcmp(val->vu.str.value, DATA_KEY) == 0)
    {
    loader->_specialLoading = JSONLoaderImpl::AttributeKey;
    }
  else if(strcmp(val->vu.str.value, NO_ROOT_KEY) == 0)
    {
    loader->_specialLoading = JSONLoaderImpl::NoRoot;
    }
  else if(strcmp(val->vu.str.value, VERSION_KEY) == 0)
    {
    loader->_specialLoading = JSONLoaderImpl::Version;
    }
  else if(strcmp(val->vu.str.value, TYPES_KEY) == 0)
    {
    loader->_specialLoading = JSONLoaderImpl::ContainedTypesKey;
    }
  else
    {
    return false;
    }

  loader->_current = JSONLoaderImpl::HeaderAwaitingValue;
  return true;
  }

bool headerVal(JSONLoaderImpl *loader, int type, const JSON_value *val)
  {
  if(loader->_specialLoading == JSONLoaderImpl::Version)
    {
    if(strcmp(val->vu.str.value, formatVersion) != 0)
      {
      return false;
      }
    }
  else if(loader->_specialLoading == JSONLoaderImpl::NoRoot)
    {
    if(strcmp(val->vu.str.value, NO_ROOT_ON) == 0)
      {
      loader->_noRoot = true;
      }
    }
  else if(loader->_specialLoading == JSONLoaderImpl::ContainedTypesKey)
    {
    loader->_current = JSONLoaderImpl::ContainedTypes;
    return true;
    }
  else if(loader->_specialLoading == JSONLoaderImpl::AttributeKey)
    {
    // create a placeholder for the data to come.
    auto &data = loader->_loadStack.createBack();

    if(loader->_noRoot)
      {
      data.attribute = loader->_interface->begin(false, nullptr, true, loader->_alloc);

      children(loader, type, val);
      }
    else
      {
      loader->_current = JSONLoaderImpl::Attribute;
      }

    return true;
    }

  loader->_current = JSONLoaderImpl::Header;
  return true;
  }

int JSONLoaderImpl::callback(void *ctx, int type, const JSON_value *value)
  {
  JSONLoaderImpl *ldr = (JSONLoaderImpl*)ctx;

  typedef const ActionFunction StateFunctions[JSON_T_MAX];

  static StateFunctions functions[MaxState] =
    {
    //  NONE    ARRAY_BEGIN   ARRAY_END     OBJECT_BEGIN       OBJECT_END          INTEGER   FLOAT   NULL   TRUE    FALSE     STRING             KEY

    //  Start
    {   err,    err,          err,          to<Header>,        err,                err,      err,    err,   err,    err,      err,               err },

    //  Header
    {   err,    err,          err,          err,               to<Start>,          err,      err,    err,   err,    err,      err,               headerKey },

    //  HeaderAwaitingValue
    {   err,    headerVal,    err,          headerVal,         err,                err,      err,    err,   err,    err,      headerVal,         err },

    //  ContainedTypes
    {   err,    err,          err,          err,               to<Header>,         err,      err,    err,   err,    err,      err,               to<ContainedTypesAwaitingValue> },

    //  ContainedTypesAwaitingValue
    {   err,    err,          err,          to<ContainedType>, err,                err,      err,    err,   err,    err,      err,               err },

    //  ContainedType
    {   err,    err,          err,          err,               to<ContainedTypes>, err,      err,    err,   err,    err,      err,               to<ContainedTypeAwaitingValue> },

    //  ContainedTypeAwaitingValue
    {   err,    err,          err,          err,               err,                err,      err,    err,   err,    err,      to<ContainedType>, err },

    //  Attribute
    {   err,    err,          err,          err,               endChild,           err,      err,    err,   err,    err,      err,               attrKey },

    //  AttributeAwaitingValue
    {   err,    err,          err,          err,               err,                err,      err,    err,   err,    err,      attrVal,           err },

    //  AttributeAwaitingChildren
    {   err,    children,     err,          children,          err,                err,      err,    err,   err,    err,      err,               err },

    //  IndexedChildren
    {   err,    err,          endChildren,  addChild,          err,                err,      err,    err,   err,    err,      addSimpleChild,    err },

    //  NamedChildren
    {   err,    err,          err,          err,               endChildren,        err,      err,    err,   err,    err,      err,               childName },

    //  NamedChildKey
    {   err,    err,          err,          addChild,          err,                err,      err,    err,   err,    err,      addSimpleChild,    err },
    };

  const StateFunctions& stateFunctions = functions[ldr->_current];

  bool result = stateFunctions[type](ldr, type, value) ? 1 : 0;
  xAssert(result);
  return result;
  }


//----------------------------------------------------------------------------------------------------------------------
// JSONLoaderImpl Impl
//----------------------------------------------------------------------------------------------------------------------
JSONLoader::JSONLoader()
  {
  }

JSONLoader::~JSONLoader()
  {
  }

void JSONLoader::load(QIODevice *device, AttributeInterface *ifc)
  {
  SProfileFunction

  Eks::TemporaryAllocator alloc(Eks::Core::temporaryAllocator());

  Eks::UniquePointer<JSONLoaderImpl> impl = alloc.createUnique<JSONLoaderImpl>(&alloc, ifc);

  while(!device->atEnd())
    {
    char nextChar;
    device->getChar(&nextChar);

    if(!JSON_parser_char(impl->_jc, nextChar))
      {
      qWarning() << "JSON_parser_char: syntax error";
      xAssertFail();
      return;
      }
    }
  }

}
