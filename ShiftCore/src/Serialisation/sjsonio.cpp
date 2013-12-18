#include "shift/Serialisation/sjsonio.h"
#include "shift/sentity.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/Changes/shandler.h"
#include "QDebug"
#include "../Serialisation/JsonParser/JSON_parser.h"
#include "Utilities/XEventLogger.h"

namespace Eks
{

class JSONWriter
  {
XProperties:
  XROByRefProperty(String, string);
  XProperty(xsize, indent, setIndent);
  XProperty(bool, niceFormatting, setNiceFormatting);

public:
  JSONWriter(Eks::AllocatorBase *alloc)
      : _string(alloc),
        _indent(0),
        _niceFormatting(false),
        _indentString(alloc),
        _vector(&_string)
    {
    }

  void addKeyValue(const char* key, const char* value)
    {
    WriteBlock b(this);

    beginNewline();

    appendQuotedEscaped(key);

    _vector->append(": ", 2);

    appendQuotedEscaped(value);
    }

private:
  void beginNewline()
    {
    xAssert(_activeBlock);

    if (_vector->length())
      {
      _vector->pushBack('\n');
      _vector->append(_indentString.data(), _indentString.size());
      }
    }

  void appendQuotedEscaped(const char* data)
    {
    xAssert(_activeBlock);

    _vector->pushBack('"');

    const char* p = data;
    while(*p)
      {
      if(*p == '"')
        {
        _vector->pushBack('\\');
        }

      _vector->pushBack(*p);
      }

    _vector->pushBack('"');
    }

  class WriteBlock
    {
  public:
    WriteBlock(JSONWriter *writer) : _writer(writer)
      {
      _oldBlock = _writer->_activeBlock;
      _writer->_activeBlock = this;

      if (!_oldBlock)
        {
        _writer->_vector->popBack();
        }
      }

    ~WriteBlock()
      {
      _writer->_activeBlock = _oldBlock;

      if (!_writer->_activeBlock)
        {
        _writer->_vector->pushBack('\0');
        }
      }

  private:
    JSONWriter *_writer;
    WriteBlock *_oldBlock;
    };

  WriteBlock *_activeBlock;

  Eks::String::BaseType::String *_vector;
  Eks::String _indentString;
  };

}

namespace Shift
{

#define PUSH_COMMA_STACK _commaStack << false;
#define POP_COMMA_STACK _commaStack.pop_back();

#define TAB_ELEMENT if(autoWhitespace()){ for(int i=0; i<_commaStack.size(); ++i) { _device->write("  "); } }

#define OPTIONAL_NEWLINE autoWhitespace() ? "\n" : "\0"

#define NEWLINE_IF_REQUIRED if(autoWhitespace() && _commaStack.size()){_device->write("\n");}

#define COMMA_IF_REQUIRED { if(_commaStack.size()) { if(_commaStack.back() == true) { _device->write(","); } else { _commaStack.back() = true; } } }

#define START_ARRAY_IN_OBJECT_CHAR(key) COMMA_IF_REQUIRED NEWLINE_IF_REQUIRED TAB_ELEMENT _device->write("\"" key "\":["); PUSH_COMMA_STACK
#define END_ARRAY NEWLINE_IF_REQUIRED POP_COMMA_STACK TAB_ELEMENT _device->write("]");

#define START_OBJECT COMMA_IF_REQUIRED NEWLINE_IF_REQUIRED TAB_ELEMENT _device->write("{"); PUSH_COMMA_STACK
#define END_OBJECT NEWLINE_IF_REQUIRED POP_COMMA_STACK TAB_ELEMENT _device->write("}");

#define OBJECT_VALUE_CHAR_BYTEARRAY(key, valueString) COMMA_IF_REQUIRED NEWLINE_IF_REQUIRED TAB_ELEMENT _device->write("\"" key "\":\""); _device->write(escape(valueString)); _device->write("\"");
#define OBJECT_VALUE_CHAR_CHAR(key, valueString) COMMA_IF_REQUIRED NEWLINE_IF_REQUIRED TAB_ELEMENT _device->write("\"" key "\":\"" valueString "\"");
#define OBJECT_VALUE_BYTEARRAY_BYTEARRAY(key, valueString) COMMA_IF_REQUIRED NEWLINE_IF_REQUIRED TAB_ELEMENT _device->write("\""); _device->write(escape(key)); _device->write("\":\""); _device->write(escape(valueString));  _device->write("\"");

#define TYPE_KEY "type"
#define CHILD_COUNT_KEY "count"
#define CHILDREN_KEY "child"
#define VALUE_KEY "val"
#define NO_ROOT_KEY "noroot"

class StringSymbol : public SerialisationSymbol
  {
public:
  StringSymbol(const char* data) : str(data)
    {
    }

  const char* str;
  };

class JSONSaver::JSONChildSaver : public Saver::ChildData
  {
public:
  JSONChildSaver(JSONSaver::JSONAttributeSaver *saver, Saver::AttributeData::ChildrenType type)
      : _owner(saver),
        _childType(type)
    {
    }

  Eks::UniquePointer<Saver::AttributeData> beginAttribute(Attribute *a);

  JSONSaver::JSONAttributeSaver *owner() { return _owner; }

private:
  Saver::AttributeData::ChildrenType _childType;
  JSONSaver::JSONAttributeSaver *_owner;
  };

class JSONSaver::JSONAttributeSaver : public Saver::AttributeData
  {
public:
  JSONAttributeSaver(SaveData *data, JSONSaver::JSONChildSaver *prev, Attribute *attr)
    : AttributeData(data, attr),
      _attrCount(0),
      _hasValueSymbolBeenWritten(false),
      _valueOnly(allocator()),
      _parent(prev)
    {
    }

  ~JSONAttributeSaver()
    {
    bool onlyHasValue = hasOnlyWrittenValueSymbol();
    if (onlyHasValue)
      {

      }
    }

  Eks::UniquePointer<Saver::ChildData> beginChildren(ChildrenType type) X_OVERRIDE
    {
    return allocator()->createUnique<JSONSaver::JSONChildSaver>(
               this, type);
    }

  void writeValue(const Symbol &id, const SerialisationValue& value) X_OVERRIDE
    {
    ++_attrCount;

    Eks::String valueStr = value.asUtf8(allocator());

    if (&id == &valueSymbol())
      {
      xAssert(!_hasValueSymbolBeenWritten);
      _hasValueSymbolBeenWritten = true;

      _valueOnly = valueStr;
      }

    writer().addKeyValue(static_cast<const StringSymbol &>(id).str, valueStr.data());
    }

  bool hasValues() { return _attrCount > 0; }
  bool hasOnlyWrittenValueSymbol() { return _attrCount == 1 && _hasValueSymbolBeenWritten; }

private:
  Eks::JSONWriter& writer();

  xsize _attrCount;
  bool _hasValueSymbolBeenWritten;

  Eks::String _valueOnly;

  JSONSaver::JSONChildSaver *_parent;
  };

class JSONSaver::Impl : public Saver::SaveData
  {
public:
  Impl(Attribute *root, Saver *saver)
    : SaveData(root, saver),
      _mode("mode"),
      _input("input"),
      _value("value"),
      _type("type"),
      _root(this, nullptr, root),
      _includeRoot(true),
      _writer(_root.allocator()),
      _typeMap(_root.allocator())
    {
    }

  void addSavedType(const PropertyInformation *info) X_OVERRIDE
    {
    _typeMap[info] = _typeMap.value(info, 0) + 1;
    }

  void setIncludeRoot(bool include) X_OVERRIDE
    {
    _includeRoot = include;
    }

  AttributeData* rootData() X_OVERRIDE
    {
    return &_root;
    }

  const SerialisationSymbol &modeSymbol() X_OVERRIDE
    {
    return _mode;
    }

  const SerialisationSymbol &inputSymbol() X_OVERRIDE
    {
    return _input;
    }

  const SerialisationSymbol &valueSymbol() X_OVERRIDE
    {
    return _value;
    }

  const SerialisationSymbol &typeSymbol() X_OVERRIDE
    {
    return _type;
    }

  StringSymbol _mode, _input, _value, _type;
  JSONSaver::JSONAttributeSaver _root;
  Eks::JSONWriter _writer;
  Eks::UnorderedMap<const PropertyInformation *, xsize> _typeMap;
  bool _includeRoot;
  };

Eks::UniquePointer<Saver::AttributeData> JSONSaver::JSONChildSaver::beginAttribute(Attribute *a)
  {
  return _owner->allocator()->createUnique<JSONSaver::JSONAttributeSaver>(
             _owner->saveData(),
             this,
             a);
  }

Eks::JSONWriter& JSONSaver::JSONAttributeSaver::writer()
  {
  return static_cast<JSONSaver::Impl*>(saveData())->_writer;
  }

JSONSaver::JSONSaver() : _autoWhitespace(false)
  {
  }

Eks::UniquePointer<JSONSaver::SaveData> JSONSaver::beginVisit(Attribute *root)
  {
  return saveAllocator()->createUnique<Impl>(root, this);
  }


JSONLoader::JSONLoader()
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
  }

}
