#ifndef SJSONIO_H
#define SJSONIO_H

#include "QBuffer"
#include "sloader.h"
#include "shift/Serialisation/ssaver.h"
#include "QXmlStreamWriter"
#include "QXmlStreamReader"
#include "Memory/XUniquePointer.h"
#include "Containers/XUnorderedMap.h"

struct JSON_value_struct;
struct JSON_parser_struct;

namespace Shift
{

class SHIFT_EXPORT JSONSaver : public Saver
  {
XProperties:
  XProperty(bool, autoWhitespace, setAutoWhitespace);

public:
  JSONSaver();

protected:
  Eks::UniquePointer<SaveData> beginVisit(Attribute *root) X_OVERRIDE;

  void beginNamedChildren(Attribute *a) X_OVERRIDE;
  void endNamedChildren(Attribute *a) X_OVERRIDE;

  void beginIndexedChildren(Attribute *a) X_OVERRIDE;
  void endIndexedChildren(Attribute *a) X_OVERRIDE;

  Eks::UniquePointer<AttributeData> beginAttribute(Attribute *a) X_OVERRIDE;

private:
  class JSONAttributeSaver;
  class Impl;
  Eks::UniquePointer<Impl> _impl;
  };

class SHIFT_EXPORT JSONLoader : private Loader
  {
public:
  JSONLoader();
  ~JSONLoader();

  void readFromDevice(QIODevice *device, Container *parent);

private:
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
