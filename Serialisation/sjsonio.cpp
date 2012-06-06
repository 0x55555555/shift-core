#include "sjsonio.h"
#include "sentity.h"
#include "spropertyinformation.h"
#include "styperegistry.h"
#include "shandler.h"
#include "QDebug"
#include "Serialisation/JsonParser/JSON_parser.h"

QByteArray escape(const QByteArray &s)
  {
  QByteArray r = s;
  r.replace("\\", "\\\\");

  r.replace('\"', "\\\"");
  r.replace('/', "\\/");
  r.replace('\b', "\\b");
  r.replace('\f', "\\f");
  r.replace('\n', "\\n");
  r.replace('\r', "\\r");
  r.replace('\t', "\\t");
  return r;
  }

#define PUSH_COMMA_STACK _commaStack << false;
#define POP_COMMA_STACK _commaStack.pop_back();

#define TAB_ELEMENT if(autoWhitespace()){ for(int i=0; i<_commaStack.size(); ++i) { _device->write("  "); } }

#define OPTIONAL_NEWLINE autoWhitespace() ? "\n" : "\0"

#define NEWLINE_IF_REQUIRED if(autoWhitespace() && _commaStack.size()){_device->write("\n");}

#define COMMA_IF_REQUIRED { if(_commaStack.size()) { if(_commaStack.back() == true) { _device->write(","); } else { _commaStack.back() = true; } } }

#define START_ARRAY_IN_OBJECT_CHAR(key) COMMA_IF_REQUIRED NEWLINE_IF_REQUIRED TAB_ELEMENT _device->write("\""key "\":["); PUSH_COMMA_STACK
#define END_ARRAY NEWLINE_IF_REQUIRED POP_COMMA_STACK TAB_ELEMENT _device->write("]");

#define START_OBJECT COMMA_IF_REQUIRED NEWLINE_IF_REQUIRED TAB_ELEMENT _device->write("{"); PUSH_COMMA_STACK
#define END_OBJECT NEWLINE_IF_REQUIRED POP_COMMA_STACK TAB_ELEMENT _device->write("}");

#define OBJECT_VALUE_CHAR_BYTEARRAY(key, valueString) COMMA_IF_REQUIRED NEWLINE_IF_REQUIRED TAB_ELEMENT _device->write("\"" key "\":\""); _device->write(escape(valueString)); _device->write("\"");
#define OBJECT_VALUE_CHAR_CHAR(key, valueString) COMMA_IF_REQUIRED NEWLINE_IF_REQUIRED TAB_ELEMENT _device->write("\"" key "\":\"" valueString "\"");
#define OBJECT_VALUE_BYTEARRAY_BYTEARRAY(key, valueString) COMMA_IF_REQUIRED NEWLINE_IF_REQUIRED TAB_ELEMENT _device->write("\""); _device->write(escape(key)); _device->write("\":\""); _device->write(escape(valueString));  _device->write("\"");

#define TYPE_KEY "__T"
#define CHILD_COUNT_KEY "__C"
#define CHILDREN_KEY "__H"
#define VALUE_KEY "__V"
#define NO_ROOT_KEY "__N"

SJSONSaver::SJSONSaver() : _autoWhitespace(false), _device(0), _root(0)
  {
  if(_autoWhitespace)
    {
    _commaStack.reserve(8);
    }
  _buffer.open(QIODevice::WriteOnly);
  setStreamDevice(Text, &_buffer);
  }

void SJSONSaver::writeToDevice(QIODevice *device, const SPropertyContainer *ent, bool includeRoot)
  {
  SProfileFunction
  _root = ent;

  _device = device;


  if(!includeRoot)
    {
    START_OBJECT
    OBJECT_VALUE_CHAR_CHAR(NO_ROOT_KEY, "true");

    saveChildren(_root);

    END_OBJECT
    }
  else
    {
    START_OBJECT
    write(_root);
    END_OBJECT
    }


  if(_autoWhitespace)
    {
    _device->write("\n");
    }

  _root = 0;
  }

void SJSONSaver::setType(const SPropertyInformation *type)
  {
  SProfileFunction
  xAssert(_buffer.data().isEmpty());
  xAssert(_inAttribute.isEmpty());

  OBJECT_VALUE_CHAR_BYTEARRAY(TYPE_KEY, type->typeName().toUtf8());
  }

void SJSONSaver::beginChildren()
  {
  SProfileFunction
  xAssert(_inAttribute.isEmpty());
  START_ARRAY_IN_OBJECT_CHAR(CHILDREN_KEY);
  }

void SJSONSaver::endChildren()
  {
  SProfileFunction
  xAssert(_inAttribute.isEmpty());
  END_ARRAY
  }

void SJSONSaver::beginNextChild()
  {
  SProfileFunction
  xAssert(_buffer.data().isEmpty());
  START_OBJECT
  }

void SJSONSaver::endNextChild()
  {
  SProfileFunction
  textStream().flush();
  if(!_buffer.buffer().isEmpty())
    {
    OBJECT_VALUE_CHAR_BYTEARRAY(VALUE_KEY, _buffer.buffer());
    _buffer.buffer().clear();
    textStream().seek(0);
    }
  END_OBJECT
  }

void SJSONSaver::beginAttribute(const char *attrName)
  {
  SProfileFunction
  xAssert(_inAttribute.isEmpty());
  _inAttribute = attrName;
  xAssert(!_inAttribute.isEmpty());

  textStream().flush();
  xAssert(_buffer.buffer().isEmpty());
  }

void SJSONSaver::endAttribute(const char *attrName)
  {
  SProfileFunction
  xAssert(!_inAttribute.isEmpty());
  xAssert(_inAttribute == attrName);

  textStream().flush();
  if(!_buffer.buffer().isEmpty())
    {
    OBJECT_VALUE_BYTEARRAY_BYTEARRAY(_inAttribute.toUtf8(), _buffer.buffer());
    _buffer.buffer().clear();
    textStream().seek(0);
    }

  _inAttribute.clear();
  }


SJSONLoader::SJSONLoader() : _current(Start)
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

SJSONLoader::~SJSONLoader()
  {
  delete_JSON_parser(_jc);
  }

int SJSONLoader::callback(void *ctx, int type, const JSON_value* value)
  {
  SProfileFunction
  SJSONLoader *ldr = (SJSONLoader*)ctx;

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

void SJSONLoader::readNext() const
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

void SJSONLoader::readAllAttributes()
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

void SJSONLoader::readFromDevice(QIODevice *device, SPropertyContainer *parent)
  {
  SBlock b(parent->handler());
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

  foreach(SProperty *prop, _resolveAfterLoad.keys())
    {
    SProperty* input = prop->resolvePath(_resolveAfterLoad.value(prop));

    xAssert(input);
    if(input)
      {
      input->connect(prop);
      }
    }

  _buffer.close();
  _root = 0;
  }


const SPropertyInformation *SJSONLoader::type() const
  {
  SProfileFunction
  xAssert(_root);

  const SPropertyInformation *info = STypeRegistry::findType(_currentAttributes.value(TYPE_KEY));
  xAssert(info);
  return info;
  }


bool SJSONLoader::beginChildren() const
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

void SJSONLoader::endChildren() const
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

bool SJSONLoader::hasNextChild() const
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

void SJSONLoader::beginNextChild()
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

bool SJSONLoader::childHasValue() const
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

void SJSONLoader::endNextChild()
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

void SJSONLoader::beginAttribute(const char *attr)
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

void SJSONLoader::endAttribute(const char *)
  {
  SProfileFunction
  _buffer.close();
  _buffer.setBuffer(&_currentValue);
  _buffer.open(QIODevice::ReadOnly);
  textStream().seek(0);

  _currentAttributeValue.clear();
  }

void SJSONLoader::resolveInputAfterLoad(SProperty *prop, const QString &path)
  {
  SProfileFunction
  _resolveAfterLoad.insert(prop, path);
  }
