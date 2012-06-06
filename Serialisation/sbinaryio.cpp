#include "sbinaryio.h"
#include "sentity.h"
#include "sdatabase.h"
#include "QIODevice"

SBinarySaver::SBinarySaver() : _device(0), _root(0)
  {
  _buffer.open(QIODevice::WriteOnly);
  setStreamDevice(Binary, &_buffer);
  }

void SBinarySaver::writeToDevice(QIODevice *device, const SEntity *ent)
  {
  _root = ent;
  _device = device;

  xuint32 version = 0;
  _device->write(reinterpret_cast<const char*>(&version), sizeof(version));

  write(_root);

  _device = 0;
  _root = 0;
  }

void SBinarySaver::setType(const SPropertyInformation *)
  {
  xAssert(_buffer.data().isEmpty());
  xAssert(_inAttribute.isEmpty());
  //_currentType = type->typeName();
  }

void SBinarySaver::beginChildren()
  {
  xAssert(_inAttribute.isEmpty());
  /*_inAttribute = "childCount";
  _writer.writeAttribute(_inAttribute, QString::number(size));
  _inAttribute.clear();*/
  }


void SBinarySaver::endChildren()
  {
  xAssert(_inAttribute.isEmpty());
  /*_inAttribute = "childCount";
  _writer.writeAttribute(_inAttribute, QString::number(size));
  _inAttribute.clear();*/
  }

void SBinarySaver::beginNextChild()
  {
  xAssert(_buffer.data().isEmpty());
  }

void SBinarySaver::endNextChild()
  {
  /*textStream().flush();
  if(!_buffer.buffer().isEmpty())
    {
    _writer.writeCharacters(_buffer.buffer());
    _buffer.buffer().clear();
    textStream().seek(0);
    }
  _writer.writeEndElement();*/
  }

void SBinarySaver::write(const SProperty *)
  {
  /*const SPropertyInformation *info = prop->typeInformation();
  xAssert(info);
  xAssert(info->save());

  info->save()(prop, *this);*/
  }

void SBinarySaver::beginAttribute(const char *attrName)
  {
  xAssert(_inAttribute.isEmpty());
  _inAttribute = attrName;
  xAssert(!_inAttribute.isEmpty());

  //textStream().flush();
  xAssert(_buffer.buffer().isEmpty());
  }

void SBinarySaver::endAttribute(const char *attrName)
  {
  xAssert(!_inAttribute.isEmpty());
  xAssert(_inAttribute == attrName);

  /*textStream().flush();
  if(!_buffer.buffer().isEmpty())
    {
    _writer.writeAttribute(_inAttribute, _buffer.buffer());
    _buffer.buffer().clear();
    textStream().seek(0);
    }*/

  _inAttribute.clear();
  }

#if 0
SXMLLoader::SXMLLoader()
  {
  _buffer.open(QIODevice::ReadOnly);

  _childCount = "childCount";

  setStreamDevice(Text, &_buffer);
  }

void SXMLLoader::readFromDevice(QIODevice *device, SEntity *parent)
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

      qDebug() << "name" << _currentAttributes.value("name");

      xsize count = childCount();
      for(xsize i=0; i<count; ++i)
        {
        beginNextChild();
        read(_root);
        endNextChild();
        }

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


const SPropertyInformation *SXMLLoader::type() const
  {
  xAssert(_root);

  const SPropertyInformation *info = _root->handler()->findType(_typeName);
  xAssert(info);
  return info;
  }


xsize SXMLLoader::childCount() const
  {
  _scratch.clear();
  _currentAttributes.value(_childCount).appendTo(&_scratch);
  return _scratch.toULongLong();
  }

void SXMLLoader::beginNextChild()
  {
  findNext(false);
  xAssert(_reader.isStartElement());

  _typeName.clear();
  _reader.name().appendTo(&_typeName);
  _currentValue.clear();
  _currentAttributes = _reader.attributes();

  qDebug() << "name" << _currentAttributes.value("name");

  findNext(true);
  if(_reader.isCharacters())
    {
    _scratch.clear();
    _reader.text().appendTo(&_scratch);
    _currentValue = _scratch.toAscii();
    }
  else
    {
    _hasNextElement = true;
    xAssert(_reader.isEndElement() || _reader.isStartElement());
    }
  }

bool childHasValue() const
  {
  xAssertFail();
  return true;
  }

void SXMLLoader::endNextChild()
  {
  findNext(false);
  xAssert(_reader.isEndElement());
  _typeName.clear();
  }

void SXMLLoader::read(SPropertyContainer *read)
  {
  _buffer.close();
  _buffer.setBuffer(&_currentValue);
  _buffer.open(QIODevice::ReadOnly);
  textStream().seek(0);

  const SPropertyInformation *info = type();
  xAssert(info);

  xAssert(info->load());

  info->load()(read, *this);
  }

void SXMLLoader::beginAttribute(const char *attr)
  {
  xAssert(_currentAttributeValue.isEmpty());
  _scratch.clear();
  _currentAttributes.value(attr).appendTo(&_scratch);
  _currentAttributeValue = _scratch.toAscii();

  qDebug() << attr << _currentAttributes.value(attr) << _currentAttributeValue;

  _buffer.close();
  _buffer.setBuffer(&_currentAttributeValue);
  _buffer.open(QIODevice::ReadOnly);
  textStream().seek(0);
  }

void SXMLLoader::endAttribute(const char *attr)
  {
  _buffer.close();
  _buffer.setBuffer(&_currentValue);
  _buffer.open(QIODevice::ReadOnly);
  textStream().seek(0);

  _currentAttributeValue.clear();
  }

void SXMLLoader::resolveInputAfterLoad(SProperty *prop, const QString &path)
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
#endif
