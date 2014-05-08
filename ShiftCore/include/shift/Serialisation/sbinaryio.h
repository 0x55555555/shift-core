#ifndef SXMLIO_H
#define SXMLIO_H

#if 0

#include "QBuffer"
#include "sloader.h"
#include "QXmlStreamWriter"
#include "QXmlStreamReader"

namespace Shift
{

/*
class SHIFT_EXPORT SBinarySaver : private Saver
  {
public:
  SBinarySaver();

  void writeToDevice(QIODevice *device, const Entity *ent);

private:
  void beginChildren();
  void endChildren();
  void beginNextChild();
  void endNextChild();

  void write(const Property *);

  void beginAttribute(const char *);
  void endAttribute(const char *);

  QIODevice* _device;
  const Entity *_root;

  QByteArray _inAttribute;
  QBuffer _buffer;
  };

class SHIFT_EXPORT SBinaryLoader : private Loader
  {
public:
  SBinaryLoader();

  void readFromDevice(QIODevice *device, Entity *parent);

private:
  const PropertyInformation *type() const;

  bool beginChildren() const;
  void endChildren() const;
  void beginNextChild();
  bool childHasValue() const;
  void endNextChild();

  void read(Container *);

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

  QHash<Property *, QString> _resolveAfterLoad;
  };*/

}

#endif

#endif // SXMLIO_H
