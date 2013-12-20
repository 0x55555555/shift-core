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
  XProperty(bool, niceFormatting, setNiceFormatting);

public:
  JSONWriter(Eks::AllocatorBase *alloc, const JSONWriter *parent = nullptr)
      : _string(alloc),
        _niceFormatting(false),
        _activeBlock(nullptr),
        _stack(alloc),
        _vector(&_string),
        _indentString(alloc)
    {
    if(parent)
      {
      _indentString = parent->_indentString;
      _niceFormatting = parent->niceFormatting();
      }
    }

  ~JSONWriter()
    {
    xAssert(_stack.size() == 0);
    }

  void clear()
    {
    _string.clear();
    }

  void tabIn()
    {
    _indentString.append("\t");
    }

  void tabOut()
    {
    _indentString.resize(_indentString.size() - 1, '\t');
    }

  void addBlock(const char *data, bool addedElementsToScope)
    {
    xAssert(data);
    WriteBlock b(this);
    beginNewlineImpl();

    _vector->append(data, strlen(data));

    if (addedElementsToScope)
      {
      xAssert(_stack.size());
      auto &obj = _stack.back();

      obj.hasElements = addedElementsToScope;
      }
    }

  struct Scope
    {
    bool hasElements;
    bool hasOpenElement;

    enum Type
      {
      Object,
      Array
      } type;
    };

  void beginArray()
    {
    beginScope(Scope::Array);
    }

  void beginObject()
    {
    beginScope(Scope::Object);
    }

  void beginScope(Scope::Type type)
    {
    WriteBlock b(this);

    if(type == Scope::Object)
      {
      _vector->pushBack('{');
      }
    else
      {
      _vector->pushBack('[');
      }

    tabIn();

    auto &obj = _stack.createBack();
    obj.hasElements = false;
    obj.hasOpenElement = false;
    obj.type = type;
    }

  void end()
    {
    WriteBlock b(this);

    xAssert(_stack.size());
    auto &obj = _stack.back();

    xAssert(!obj.hasOpenElement);
    tabOut();

    if(obj.hasElements)
      {
      beginNewlineImpl();
      }

    if(_stack.back().type == Scope::Object)
      {
      _vector->pushBack('}');
      }
    else
      {
      _vector->pushBack(']');
      }

    _stack.popBack();
    }

  void beginObjectElement(const char *key)
    {
    WriteBlock b(this);

    xAssert(_stack.size());
    xAssert(_stack.back().type == Scope::Object);
    xAssert(_stack.back().hasOpenElement == false);

    if(_stack.back().hasElements)
      {
      _vector->pushBack(',');
      }

    addKey(key);
    _stack.back().hasOpenElement = true;
    }

  void beginArrayElement()
    {
    xAssert(_stack.size())
    xAssert(_stack.back().type == Scope::Array);
    xAssert(_stack.back().hasOpenElement == false);
    
    WriteBlock b(this);

    if(_stack.back().hasElements)
      {
      _vector->pushBack(',');
      }

    beginNewlineImpl();
    _stack.back().hasOpenElement = true;
    }

  void endElement()
    {
    xAssert(_stack.size());

    xAssert(_stack.back().hasOpenElement == true);
    _stack.back().hasElements = true;
    _stack.back().hasOpenElement = false;
    }

  void addValueForElement(const char *data)
    {
    xAssert(data);
    WriteBlock b(this);

    appendQuotedEscaped(data);

    xAssert(_stack.size());
    _stack.back().hasElements = true;
    }

  void addKeyValueStandalone(const char *key, const char *value, bool commaFirst)
    {
    WriteBlock b(this);

    if(commaFirst)
      {
      _vector->pushBack(',');
      }

    addKey(key);

    appendQuotedEscaped(value);
    }

  void beginNewline()
    {
    WriteBlock b(this);
    beginNewlineImpl();
    }

private:
  void beginNewlineImpl()
    {
    xAssert(_activeBlock);

    if (_niceFormatting && _vector->length())
      {
      _vector->pushBack('\n');
      _vector->append(_indentString.data(), _indentString.size());
      }
    }

  void addKey(const char *key)
    {
    WriteBlock b(this);

    beginNewlineImpl();

    appendQuotedEscaped(key);

    if(_niceFormatting)
      {
      _vector->append(": ", 2);
      }
    else
      {
      _vector->pushBack(':');
      }
    }

  void appendQuotedEscaped(const char* data)
    {
    xAssert(_activeBlock);

    _vector->pushBack('"');


    for(const char* p = data; *p; ++p)
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

      if (!_oldBlock && _writer->_vector->size())
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

  Eks::Vector<Scope> _stack;

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
  JSONChildSaver(JSONSaver::JSONAttributeSaver *saver, Saver::AttributeData::ChildrenType type);
  ~JSONChildSaver();

  void addChild(JSONSaver::JSONAttributeSaver *child);
  void childComplete();

  bool isNamed() const { return _childType == Saver::AttributeData::Named; }

  Eks::UniquePointer<Saver::AttributeData> beginAttribute(Attribute *a);

  JSONSaver::JSONAttributeSaver *owner() { return _owner; }

private:
  Saver::AttributeData::ChildrenType _childType;
  JSONSaver::JSONAttributeSaver *_owner;

  JSONSaver::JSONAttributeSaver *_activeChild;
  };

//----------------------------------------------------------------------------------------------------------------------
// JSONSaver::JSONValueSaver Def
//----------------------------------------------------------------------------------------------------------------------
class JSONSaver::JSONValueSaver : public Saver::ValueData
  {
public:
  JSONValueSaver(JSONSaver::JSONAttributeSaver *saver);
  ~JSONValueSaver();

  void writeValue(const Symbol &id, const SerialisationValue& value) X_OVERRIDE
    {

    Eks::String valueStr = value.asUtf8(owner()->allocator());

    if (&id == &valueSymbol())
      {
      xAssert(!_hasValueSymbolBeenWritten);
      _hasValueSymbolBeenWritten = true;

      _valueOnly = valueStr;
      }

    _allValuesWriter.addKeyValueStandalone(static_cast<const StringSymbol &>(id).str, valueStr.data(), _attrCount != 0);

    ++_attrCount;
    }

  bool hasValues() { return _attrCount > 0; }
  bool hasOnlyWrittenValueSymbol() { return _attrCount == 1 && _hasValueSymbolBeenWritten; }


  const Eks::String& valueSymbolDataOnly() { return _valueOnly; }
  const Eks::String& allValues() { return _allValuesWriter.string(); }

private:
  Eks::JSONWriter _allValuesWriter;

  xsize _attrCount;
  bool _hasValueSymbolBeenWritten;

  Eks::String _valueOnly;
  };

//----------------------------------------------------------------------------------------------------------------------
// JSONSaver::JSONAttributeSaver Def
//----------------------------------------------------------------------------------------------------------------------
class JSONSaver::JSONAttributeSaver : public Saver::AttributeData
  {
public:
  JSONAttributeSaver(SaveData *data, JSONSaver::JSONChildSaver *prev, Attribute *attr)
    : AttributeData(data, attr),
      _values(nullptr),
      _children(nullptr),
      _hasValues(false),
      _hasChildren(false),
      _hasBegunObject(false),
      _includeRoot(true),
      _parent(prev)
    {
    if(_parent)
      {
      _parent->addChild(this);
      }
    }

  ~JSONAttributeSaver()
    {
    complete();

    if(_parent)
      {
      _parent->childComplete();
      }
    }

  void complete()
    {
    if(_hasBegunObject)
      {
      writer()->end();
      }
    }

  bool includesRoot() const
    {
    return _includeRoot;
    }

  void setIncludeRoot(bool include)
    {
    xAssert(!_hasBegunObject);
    xAssert(!_parent);
    _includeRoot = include;
    }

  Eks::UniquePointer<Saver::ChildData> beginChildren(ChildrenType type) X_OVERRIDE
    {
    xAssert(!_hasChildren);
    xAssert(!_children);
    xAssert(attribute()->castTo<Container>());

    return allocator()->createUnique<JSONSaver::JSONChildSaver>(this, type);
    }

  Eks::UniquePointer<Saver::ValueData> beginValues() X_OVERRIDE
    {
    xAssert(!_hasValues);
    xAssert(!_values);
    return allocator()->createUnique<JSONSaver::JSONValueSaver>(this);
    }

  void setValues(JSONSaver::JSONValueSaver *vals)
    {
    xAssert(!_values);
    _values = vals;
    }

  void valuesComplete()
    {
    xAssert(_values);
    if(_includeRoot)
      {
      if(_values->hasOnlyWrittenValueSymbol() && !attribute()->castTo<Container>())
        {
        writer()->addValueForElement(_values->valueSymbolDataOnly().data());
        }
      else
        {
        writer()->beginObject();
        _hasBegunObject = true;

        if(_values->hasValues())
          {
          writer()->addBlock(_values->allValues().data(), true);
          }
        }
      }

    _hasValues = true;
    _values = nullptr;
    }

  void setChildren(JSONSaver::JSONChildSaver *vals);
  void childrenComplete();

  Eks::JSONWriter* writer();

private:
  JSONSaver::JSONValueSaver *_values;
  JSONSaver::JSONChildSaver *_children;

  bool _hasValues;
  bool _hasChildren;

  bool _hasBegunObject;

  bool _includeRoot;

  JSONSaver::JSONChildSaver *_parent;
  };

//----------------------------------------------------------------------------------------------------------------------
// JSONSaver::Impl Def
//----------------------------------------------------------------------------------------------------------------------
class JSONSaver::Impl : public Saver::SaveData
  {
public:
  Impl(Attribute *root, Saver *saver)
    : SaveData(saver),
      _mode("mode"),
      _input("input"),
      _value("value"),
      _type("type"),
      _children("contents"),
      _alloc(root->temporaryAllocator()),
      _writer(&_alloc),
      _root(this, nullptr, root),
      _typeMap(&_alloc)
    {
    _writer.setNiceFormatting(static_cast<JSONSaver*>(saver)->autoWhitespace());
    _writer.tabIn();
    }

  ~Impl()
    {
    _root.complete();
    emitJson(saver()->activeBlock()->device());
    }

  void emitJson(QIODevice *dev)
    {
    Eks::TemporaryAllocator alloc(_root.attribute()->temporaryAllocator());
    Eks::JSONWriter writer(&alloc);
    writer.setNiceFormatting(_writer.niceFormatting());

    writer.beginObject();
    
    writer.beginObjectElement("version");
    writer.addValueForElement("2");
    writer.endElement();

    writer.beginObjectElement("noRoot");
    writer.addValueForElement(_root.includesRoot() ? "0" : "1");
    writer.endElement();

    writer.beginObjectElement("data");

    dev->write(writer.string().data(), writer.string().length());
    dev->write(_writer.string().data());
    if(_writer.niceFormatting())
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

  void addSavedType(const PropertyInformation *info) X_OVERRIDE
    {
    _typeMap[info] = _typeMap.value(info, 0) + 1;
    }

  void setIncludeRoot(bool include) X_OVERRIDE
    {
    _root.setIncludeRoot(include);
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

  const SerialisationSymbol &childrenSymbol()
    {
    return _children;
    }

  StringSymbol _mode, _input, _value, _type, _children;
  Eks::TemporaryAllocator _alloc;
  Eks::JSONWriter _writer;
  JSONSaver::JSONAttributeSaver _root;
  Eks::UnorderedMap<const PropertyInformation *, xsize> _typeMap;
  };

//----------------------------------------------------------------------------------------------------------------------
// JSONSaver::JSONChildSaver Impl
//----------------------------------------------------------------------------------------------------------------------
JSONSaver::JSONChildSaver::JSONChildSaver(JSONSaver::JSONAttributeSaver *saver, Saver::AttributeData::ChildrenType type)
    : _childType(type),
      _owner(saver),
      _activeChild(nullptr)
  {
  _owner->setChildren(this);
  }

JSONSaver::JSONChildSaver::~JSONChildSaver()
  {
  _owner->childrenComplete();
  }

void JSONSaver::JSONChildSaver::addChild(JSONSaver::JSONAttributeSaver *child)
  {
  xAssert(!_activeChild);

  if(isNamed())
    {
    _owner->writer()->beginObjectElement(child->attribute()->name().data());
    }
  else
    {
    _owner->writer()->beginArrayElement();
    }

  _activeChild = child;
  }

void JSONSaver::JSONChildSaver::childComplete()
  {
  xAssert(_activeChild);

  _owner->writer()->endElement();

  _activeChild = nullptr;
  }

Eks::UniquePointer<Saver::AttributeData> JSONSaver::JSONChildSaver::beginAttribute(Attribute *a)
  {
  return _owner->allocator()->createUnique<JSONSaver::JSONAttributeSaver>(
             _owner->saveData(),
             this,
             a);
  }

//----------------------------------------------------------------------------------------------------------------------
// JSONSaver::JSONValueSaver Impl
//----------------------------------------------------------------------------------------------------------------------
JSONSaver::JSONValueSaver::JSONValueSaver(JSONSaver::JSONAttributeSaver *saver)
    : ValueData(saver),
      _allValuesWriter(saver->allocator(), saver->writer()),
      _attrCount(0),
      _hasValueSymbolBeenWritten(false),
      _valueOnly(saver->allocator())
  {
  _allValuesWriter.tabIn();
  saver->setValues(this);
  }

JSONSaver::JSONValueSaver::~JSONValueSaver()
  {
  static_cast<JSONSaver::JSONAttributeSaver *>(owner())->valuesComplete();
  }

//----------------------------------------------------------------------------------------------------------------------
// JSONSaver::JSONAttributeSaver Impl
//----------------------------------------------------------------------------------------------------------------------
void JSONSaver::JSONAttributeSaver::setChildren(JSONSaver::JSONChildSaver *vals)
  {
  xAssert(!_children);
  xAssert(_hasValues);


  if(_includeRoot)
    {
    const Symbol& sym = static_cast<JSONSaver::Impl*>(saveData())->childrenSymbol();
    writer()->beginObjectElement(static_cast<const StringSymbol&>(sym).str);
    }

  _children = vals;
  if(_children->isNamed())
    {
    writer()->beginObject();
    }
  else
    {
    writer()->beginArray();
    }
  }

void JSONSaver::JSONAttributeSaver::childrenComplete()
  {
  xAssert(_children);

  if(_children->isNamed())
    {
    writer()->end();
    }
  else
    {
    writer()->end();
    }

  if(_includeRoot)
    {
    writer()->endElement();
    }

  _hasChildren = true;
  _children = nullptr;
  }

Eks::JSONWriter* JSONSaver::JSONAttributeSaver::writer()
  {
  return &static_cast<JSONSaver::Impl*>(saveData())->_writer;
  }

//----------------------------------------------------------------------------------------------------------------------
// JSONSaver Impl
//----------------------------------------------------------------------------------------------------------------------
JSONSaver::JSONSaver() : _autoWhitespace(false)
  {
  }

Eks::UniquePointer<JSONSaver::SaveData> JSONSaver::beginVisit(Attribute *root)
  {
  saveAllocator()->init(root->temporaryAllocator());
  return saveAllocator()->createUnique<Impl>(root, this);
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
