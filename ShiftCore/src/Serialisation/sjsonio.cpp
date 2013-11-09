#include "shift/Serialisation/sjsonio.h"
#include "shift/sentity.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/Changes/shandler.h"
#include "QDebug"
#include "../Serialisation/JsonParser/JSON_parser.h"
#include "XEventLogger"

namespace Shift
{

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

JSONSaver::JSONSaver() : _autoWhitespace(false), _device(0), _root(0)
  {
  if(_autoWhitespace)
    {
    _commaStack.reserve(8);
    }
  _buffer.open(QIODevice::WriteOnly);
  setStreamDevice(Text, &_buffer);
  }

void JSONSaver::writeToDevice(QIODevice *device, const Container *ent, bool includeRoot)
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

void JSONSaver::beginChildren()
  {
  SProfileFunction
  xAssert(_inAttribute.isEmpty());
  START_ARRAY_IN_OBJECT_CHAR(CHILDREN_KEY);
  }

void JSONSaver::endChildren()
  {
  SProfileFunction
  xAssert(_inAttribute.isEmpty());
  END_ARRAY
  }

void JSONSaver::beginNextChild()
  {
  SProfileFunction
  xAssert(_buffer.data().isEmpty());
  START_OBJECT
  }

void JSONSaver::endNextChild()
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

void JSONSaver::beginAttribute(const char *attrName)
  {
  SProfileFunction
  xAssert(_inAttribute.isEmpty());
  _inAttribute = attrName;
  xAssert(!_inAttribute.isEmpty());

  textStream().flush();
  xAssert(_buffer.buffer().isEmpty());
  }

void JSONSaver::endAttribute(const char *attrName)
  {
  SProfileFunction
  (void)attrName;
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


JSONLoader::JSONLoader() : _current(Start)
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
