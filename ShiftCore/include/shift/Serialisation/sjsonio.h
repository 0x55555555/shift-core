#ifndef SJSONIO_H
#define SJSONIO_H

#include "QBuffer"
#include "sloader.h"
#include "QXmlStreamWriter"
#include "QXmlStreamReader"
#include "XUnorderedMap"

struct JSON_value_struct;
struct JSON_parser_struct;

namespace Shift
{

class SHIFT_EXPORT SJSONSaver : private Saver
  {
XProperties:
  XProperty(bool, autoWhitespace, setAutoWhitespace);

public:
  SJSONSaver();

  void writeToDevice(QIODevice *device, const Container *ent, bool includeRoot);

private:
  void setType(const PropertyInformation *);

  void beginChildren();
  void endChildren();
  void beginNextChild();
  void endNextChild();

  void beginAttribute(const char *);
  void endAttribute(const char *);

  QIODevice *_device;
  const Container *_root;

  QVector <bool> _commaStack;

  QString _inAttribute;
  QBuffer _buffer;
  };

class SHIFT_EXPORT SJSONLoader : private Loader
  {
public:
  SJSONLoader();
  ~SJSONLoader();

  void readFromDevice(QIODevice *device, Container *parent);

private:
  const PropertyInformation *type() const;

  bool beginChildren() const;
  void endChildren() const;
  bool hasNextChild() const;
  void beginNextChild();
  bool childHasValue() const;
  void endNextChild();

  void beginAttribute(const char *);
  void endAttribute(const char *);

  virtual void resolveInputAfterLoad(Property *, const InputString &) X_OVERRIDE;

  void readAllAttributes();
  void readNext() const;
  static int callback(void *ctx, int type, const JSON_value_struct* value);

  mutable bool _parseError;
  JSON_parser_struct* _jc;
  mutable QIODevice *_device;
  Container *_root;
  QHash <QString, QByteArray> _currentAttributes;
  QByteArray _currentValue;

  QByteArray _currentAttributeValue;


  enum State
    {
    Start,
    Attributes,
    AttributesEnd,
    Children,
    ChildrenEnd,
    End
    };

  mutable bool _readNext;
  State _current;
  QString _currentKey;

  mutable QString _scratch;

  QBuffer _buffer;

  Eks::UnorderedMap<Property *, InputString> _resolveAfterLoad;
  };

}

#endif // SJSONIO_H
