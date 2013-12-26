#include "shift/Serialisation/sjsonio.h"
#include "shift/sentity.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/Changes/shandler.h"
#include "QDebug"
#include "../Serialisation/JsonParser/JSON_parser.h"
#include "Utilities/XEventLogger.h"
#include "Utilities/XJsonWriter.h"

namespace Shift
{

#define VERSION_KEY "version"
#define NO_ROOT_KEY "noroot"
#define TYPES_KEY "types"
#define DATA_KEY "data"
#define DYNAMIC_COUNT_KEY "dynamicCount"

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
    : _mode("mode"),
      _input("input"),
      _value("value"),
      _type("type"),
      _children("contents"),
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

void JSONSaver::onBegin(AttributeData *root, Eks::AllocatorBase *alloc)
  {
  Saver::onBegin(root, alloc);
  _data = alloc->createUnique<CurrentSaveData>(alloc);

  _data->_writer.setNiceFormatting(autoWhitespace());
  _data->_writer.tabIn();
  _data->_allocator = alloc;
  }

void JSONSaver::onEnd(AttributeData *root)
  {
  completeAttribute(root->as<JSONAttributeSaver>());
  emitJson(_data->_allocator, root, activeBlock()->device());

  _data = nullptr;

  Saver::onEnd(root);
  }

void JSONSaver::emitJson(Eks::AllocatorBase *allocator, AttributeData *rootData, QIODevice *dev)
  {
  Eks::JSONWriter writer(allocator);
  writer.setNiceFormatting(_data->_writer.niceFormatting());

  writer.beginObject();

  writer.beginObjectElement(VERSION_KEY);
  writer.addValueForElement("2");
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
  writer.addValueForElement(root->includeRoot() ? "0" : "1");
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

void JSONSaver::setIncludeRoot(AttributeData *root, bool include)
  {
  root->as<JSONAttributeSaver>()->setIncludeRoot(include);
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

  values->allValuesWriter.addKeyValueStandalone(static_cast<const StringSymbol &>(id).str, valueStr.data(), values->attrCount != 0);

  xAssert(values->allValuesWriter.string().data());
  ++values->attrCount;
  }


//----------------------------------------------------------------------------------------------------------------------
// JSONLoader Impl
//----------------------------------------------------------------------------------------------------------------------
/*JSONLoader::JSONLoader()
    : _current(Start)
  {
  _buffer.open(QIODevice::ReadOnly);

  setStreamDevice(Text, &_buffer);

  JSON_config config;

  init_JSON_config(&config);

  config.depth = 19;
  config.callback = &callback;
  config.callback_ctx = this;
  config.allow_comments = 1;
  config.handle_floats_manually = 0;

  _jc = new_JSON_parser(&config);
  }

JSONLoader::~JSONLoader()
  {
  delete_JSON_parser(_jc);
  }

int JSONLoader::callback(void *ctx, int type, const JSON_value* value)
  {
  SProfileFunction
  JSONLoader *ldr = (JSONLoader*)ctx;

  if(ldr->_current == Start)
    {
    if(type != JSON_T_OBJECT_BEGIN)
      {
      return 0;
      }

    ldr->_current = Attributes;
    }
  else if(ldr->_current == Attributes)
    {
    if(type == JSON_T_KEY)
      {
      xAssert(ldr->_currentKey.isEmpty());
      ldr->_currentKey = value->vu.str.value;
      if(ldr->_currentKey == CHILDREN_KEY)
        {
        ldr->_currentKey.clear();
        ldr->_current = AttributesEnd;
        }
      }
    else if(type == JSON_T_OBJECT_END)
      {
      xAssert(ldr->_currentKey.isEmpty());
      ldr->_current = End;
      }
    else
      {
      xAssert(!ldr->_currentKey.isEmpty());
      xAssert(type == JSON_T_STRING);
      ldr->_currentAttributes[ldr->_currentKey] = value->vu.str.value;
      ldr->_currentKey.clear();
      }
    }
  else if(ldr->_current == AttributesEnd)
    {
    if(type != JSON_T_ARRAY_BEGIN)
      {
      xAssertFail();
      return 0;
      }
    ldr->_current = Children;
    }
  else if(ldr->_current == Children)
    {
    if(type == JSON_T_OBJECT_BEGIN)
      {
      ldr->_current = Attributes;
      }
    else if(type == JSON_T_ARRAY_END)
      {
      ldr->_current = ChildrenEnd;
      }
    else
      {
      xAssertFail();
      return 0;
      }
    }
  else if(ldr->_current == ChildrenEnd)
    {
    if(type != JSON_T_OBJECT_END)
      {
      xAssertFail();
      return 0;
      }
    ldr->_current = End;
    }
  else if(ldr->_current == End)
    {
    if(type != JSON_T_ARRAY_END && type != JSON_T_OBJECT_BEGIN)
      {
      xAssertFail();
      return 0;
      }

    if(type == JSON_T_OBJECT_BEGIN)
      {
      ldr->_current = Attributes;
      }
    if(type == JSON_T_ARRAY_END)
      {
      ldr->_current = ChildrenEnd;
      }
    }
  else
    {
    xAssertFail();
    return 0;
    }

  ldr->_readNext = true;

  return 1;
  }

void JSONLoader::readNext() const
  {
  SProfileFunction
  xAssert(_parseError == false);
  _readNext = false;
  while(!_device->atEnd() && !_readNext)
    {
    char nextChar;
    _device->getChar(&nextChar);

    if(!JSON_parser_char(_jc, nextChar))
      {
      _parseError = true;
      qWarning() << "JSON_parser_char: syntax error";
      xAssertFail();
      return;
      }
    }
  }

void JSONLoader::readAllAttributes()
  {
  SProfileFunction
  xAssert(_current == Attributes);
  while(_current != AttributesEnd && _current != End)
    {
    readNext();
    if(_parseError)
      {
      break;
      }
    }
  }

void JSONLoader::readFromDevice(QIODevice *device, Container *parent)
  {
  Block b(parent->handler());
  SProfileFunction
  _root = parent;

  _device = device;
  _parseError = false;

  _current = Start;
  readNext();
  readAllAttributes();

  if(!_currentAttributes.contains(NO_ROOT_KEY))
    {
    read(_root);
    }
  else
    {
    loadChildren(_root);
    }

  xAssert(_current == End);

  auto it = _resolveAfterLoad.begin();
  auto end = _resolveAfterLoad.end();
  for(; it != end; ++it)
    {
    Property *prop = it.key();
    Attribute* input = prop->resolvePath(it.value());

    xAssert(input);
    if(input)
      {
      if(Property *inputProp = input->castTo<Property>())
        {
        inputProp->connect(prop);
        }
      }
    }

  _buffer.close();
  _root = 0;
  }

bool JSONLoader::beginChildren() const
  {
  SProfileFunction
  if(_current == AttributesEnd)
    {
    readNext();
    }

  if(_current == Children)
    {
    return true;
    }
  else if(_current == End)
    {
    return false;
    }

  xAssertFail();
  return false;
  }

void JSONLoader::endChildren() const
  {
  SProfileFunction
  if(_current == Children)
    {
    readNext();
    }
  xAssert(_current == ChildrenEnd);
  readNext();
  xAssert(_current == End);
  }

bool JSONLoader::hasNextChild() const
  {
  SProfileFunction
  if(_current == Children)
    {
    readNext();
    }

  if(_current == ChildrenEnd)
    {
    return false;
    }

  if(_current == Attributes)
    {
    return true;
    }


  xAssertFail();
  return false;
  }

void JSONLoader::beginNextChild()
  {
  _currentAttributes.clear();
  xAssert(_current != ChildrenEnd);
  xAssert(_current != Children);
  readAllAttributes();

  _currentValue = _currentAttributes.value(VALUE_KEY);

  _buffer.close();
  _buffer.setBuffer(&_currentValue);
  _buffer.open(QIODevice::ReadOnly);
  textStream().seek(0);
  }

bool JSONLoader::childHasValue() const
  {
  if(!_currentValue.isEmpty())
    {
    return true;
    }

  if(_current == AttributesEnd)
    {
    readNext();
    }

  if(_current == Children)
    {
    return true;
    }

  return false;
  }

void JSONLoader::endNextChild()
  {
  SProfileFunction
  if(_current == AttributesEnd)
    {
    readNext();
    xAssert(_current == Children);
    readNext();
    }
  if(_current == End)
    {
    readNext();
    }
  xAssert(_current == ChildrenEnd || _current == Attributes);
  }

void JSONLoader::beginAttribute(const char *attr)
  {
  SProfileFunction
  xAssert(_currentAttributeValue.isEmpty());
  _scratch.clear();
  _currentAttributeValue = _currentAttributes.value(attr);

  _buffer.close();
  _buffer.setBuffer(&_currentAttributeValue);
  _buffer.open(QIODevice::ReadOnly);
  textStream().seek(0);
  }

void JSONLoader::endAttribute(const char *)
  {
  SProfileFunction
  _buffer.close();
  _buffer.setBuffer(&_currentValue);
  _buffer.open(QIODevice::ReadOnly);
  textStream().seek(0);

  _currentAttributeValue.clear();
  }

void JSONLoader::resolveInputAfterLoad(Property *prop, const InputString &path)
  {
  SProfileFunction
  _resolveAfterLoad.insert(prop, path);
  }*/

}
