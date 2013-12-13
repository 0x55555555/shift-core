#ifndef SXMLIO_H
#define SXMLIO_H

#include "QBuffer"
#include "sloader.h"
#include "QXmlStreamWriter"
#include "QXmlStreamReader"
#include "Containers/XUnorderedMap.h"

namespace Shift
{

class SHIFT_EXPORT SXMLSaver : private Saver
  {
public:
  SXMLSaver();

  void writeToDevice(QIODevice *device, const Entity *ent);

private:
  void beginChildren();
  void endChildren();
  void beginNextChild();
  void endNextChild();

  void beginAttribute(const char *);
  void endAttribute(const char *);

  QXmlStreamWriter _writer;
  const Entity *_root;

  QString _inAttribute;
  QBuffer _buffer;
  };

class SHIFT_EXPORT SXMLLoader : private Loader
  {
public:
  SXMLLoader();

  void readFromDevice(QIODevice *device, Entity *parent);

private:
  const PropertyInformation *type() const;

  bool beginChildren() const;
  void endChildren() const;
  void beginNextChild();
  bool childHasValue() const;
  void endNextChild();

  void beginAttribute(const char *);
  void endAttribute(const char *);

  virtual void resolveInputAfterLoad(Property *, const InputString &) X_OVERRIDE;

  void findNext(bool allowWhitespace);
  bool isValidElement() const;

  mutable QXmlStreamReader _reader;
  Entity *_root;
  QXmlStreamAttributes _currentAttributes;
  QByteArray _currentValue;

  QByteArray _currentAttributeValue;
  QString _typeName;
  QString _childCount;

  bool _hasNextElement;
  mutable QString _scratch;

  QBuffer _buffer;

  Eks::UnorderedMap<Property *, InputString> _resolveAfterLoad;
  };

}

#endif // SXMLIO_H
