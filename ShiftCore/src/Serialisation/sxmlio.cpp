#include "shift/Serialisation//sxmlio.h"
#include "shift/sentity.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertyinformation.h"

namespace Shift
{
/*
SXMLSaver::SXMLSaver() : _writer(), _root(0)
  {
  _buffer.open(QIODevice::WriteOnly);
  setStreamDevice(Text, &_buffer);
  }

void SXMLSaver::writeToDevice(QIODevice *device, const Entity *ent)
  {
  _root = ent;

  _writer.setDevice(device);

  _writer.setAutoFormatting(true);
  _writer.setAutoFormattingIndent(2);
  _writer.writeStartDocument();

  write(_root);

  _writer.writeEndDocument();

  _root = 0;
  }

void SXMLSaver::beginChildren()
  {
  xAssert(_inAttribute.isEmpty());
  _inAttribute = "childCount";
  //_writer.writeAttribute(_inAttribute, QString::number(size));
  _inAttribute.clear();
  xAssertFail();
  }

void SXMLSaver::endChildren()
  {
  }

void SXMLSaver::beginNextChild()
  {
  xAssert(_buffer.data().isEmpty());
  }

void SXMLSaver::endNextChild()
  {
  textStream().flush();
  if(!_buffer.buffer().isEmpty())
    {
    _writer.writeCharacters(_buffer.buffer());
    _buffer.buffer().clear();
    textStream().seek(0);
    }
  _writer.writeEndElement();
  }

void SXMLSaver::beginAttribute(const char *attrName)
  {
  xAssert(_inAttribute.isEmpty());
  _inAttribute = attrName;
  xAssert(!_inAttribute.isEmpty());

  textStream().flush();
  xAssert(_buffer.buffer().isEmpty());
  }

void SXMLSaver::endAttribute(const char *attrName)
  {
  (void)attrName;
  xAssert(!_inAttribute.isEmpty());
  xAssert(_inAttribute == attrName);

  textStream().flush();
  if(!_buffer.buffer().isEmpty())
    {
    _writer.writeAttribute(_inAttribute, _buffer.buffer());
    _buffer.buffer().clear();
    textStream().seek(0);
    }

  _inAttribute.clear();
  }
*/
SXMLLoader::SXMLLoader()
  {
  _buffer.open(QIODevice::ReadOnly);

  _childCount = "childCount";

  setStreamDevice(Text, &_buffer);
  }

void SXMLLoader::readFromDevice(QIODevice *device, Entity *parent)
  {
  _hasNextElement = false;
  _root = parent;

  _reader.setDevice(device);

  while (!_reader.atEnd())
    {
    findNext(false);

    if(_reader.isStartDocument())
      {
      findNext(false);
      xAssert(_reader.isStartElement());

      _currentAttributes = _reader.attributes();

      loadChildren(_root);

      findNext(false);
      xAssert(_reader.isEndElement());
      findNext(false);
      xAssert(_reader.isEndDocument());
      }
    }

  if (_reader.hasError())
    {
    xAssertFail();
    }

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


const PropertyInformation *SXMLLoader::type() const
  {
  xAssert(_root);

  const PropertyInformation *info = TypeRegistry::findType(_typeName);
  xAssert(info);
  return info;
  }

bool SXMLLoader::beginChildren() const
  {
  _scratch.clear();
  xAssertFail(); // fix return value
  return false;
  }

void SXMLLoader::endChildren() const
  {
  }

void SXMLLoader::beginNextChild()
  {
  findNext(false);
  xAssert(_reader.isStartElement());

  _typeName.clear();
  _reader.name().appendTo(&_typeName);
  _currentValue.clear();
  _currentAttributes = _reader.attributes();

  findNext(true);
  if(_reader.isCharacters())
    {
    _scratch.clear();
    _reader.text().appendTo(&_scratch);
    _currentValue = _scratch.toUtf8();
    }
  else if(!_reader.isWhitespace())
    {
    _hasNextElement = true;

    xAssert(_reader.isEndElement() || _reader.isStartElement());
    }

  _buffer.close();
  _buffer.setBuffer(&_currentValue);
  _buffer.open(QIODevice::ReadOnly);
  textStream().seek(0);
  }

bool SXMLLoader::childHasValue() const
  {
  return !_currentValue.isEmpty();
  }

void SXMLLoader::endNextChild()
  {
  findNext(false);
  xAssert(_reader.isEndElement());
  _typeName.clear();
  }

void SXMLLoader::beginAttribute(const char *attr)
  {
  xAssert(_currentAttributeValue.isEmpty());
  _scratch.clear();
  _currentAttributes.value(attr).appendTo(&_scratch);
  _currentAttributeValue = _scratch.toUtf8();

  _buffer.close();
  _buffer.setBuffer(&_currentAttributeValue);
  _buffer.open(QIODevice::ReadOnly);
  textStream().seek(0);
  }

void SXMLLoader::endAttribute(const char *)
  {
  _buffer.close();
  _buffer.setBuffer(&_currentValue);
  _buffer.open(QIODevice::ReadOnly);
  textStream().seek(0);

  _currentAttributeValue.clear();
  }

void SXMLLoader::resolveInputAfterLoad(Property *prop, const InputString &path)
  {
  _resolveAfterLoad.insert(prop, path);
  }


bool SXMLLoader::isValidElement() const
  {
  return !_reader.isComment() && !_reader.isWhitespace();
  }

void SXMLLoader::findNext(bool allowWhitespace)
  {
  if(_hasNextElement && isValidElement())
    {
    _hasNextElement = false;

    if(allowWhitespace && _reader.isWhitespace())
      {
      xAssert(isValidElement() || (allowWhitespace && _reader.isWhitespace()));
      return;
      }
    else if(!allowWhitespace && !_reader.isWhitespace())
      {
      xAssert(isValidElement() || (allowWhitespace && _reader.isWhitespace()));
      return;
      }
    }

  do
    {
    _reader.readNext();
    } while(!isValidElement() && (!allowWhitespace && _reader.isWhitespace()));

  xAssert(isValidElement() || (allowWhitespace && _reader.isWhitespace()));
  }

}
