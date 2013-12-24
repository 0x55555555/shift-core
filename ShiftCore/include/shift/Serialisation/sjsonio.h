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

private:
  void emitJson(QIODevice *dev);
  void addSavedType(const PropertyInformation *info, bool dynamic) X_OVERRIDE;
  void setIncludeRoot(bool include) X_OVERRIDE;
  const SerialisationSymbol &modeSymbol() X_OVERRIDE;
  const SerialisationSymbol &inputSymbol() X_OVERRIDE;
  const SerialisationSymbol &valueSymbol() X_OVERRIDE;
  const SerialisationSymbol &typeSymbol() X_OVERRIDE;
  const SerialisationSymbol &childrenSymbol();
  void onBeginSave(Attribute *root, Eks::AllocatorBase *alloc) X_OVERRIDE;
  void onEndSave() X_OVERRIDE;
  Eks::UniquePointer<Saver::ChildData> onBeginChildren(AttributeData *data, Saver::ChildrenType type, Eks::AllocatorBase *alloc) X_OVERRIDE;
  void onChildrenComplete(AttributeData *data, ChildData *) X_OVERRIDE;
  Eks::UniquePointer<Saver::AttributeData> onAddChild(Saver::ChildData *parent, Attribute *a, Eks::AllocatorBase *alloc) X_OVERRIDE;
  void onChildComplete(Saver::ChildData *, AttributeData *child) X_OVERRIDE;
  void completeAttribute(AttributeData *data);
  Eks::UniquePointer<Saver::ValueData> onBeginValues(AttributeData *, Eks::AllocatorBase *alloc) X_OVERRIDE;
  void onValuesComplete(AttributeData *data, ValueData *v) X_OVERRIDE;
  void onWriteValue(Saver::ValueData *v, const Symbol &id, const SerialisationValue& value) X_OVERRIDE;

  class JSONValueSaver;
  class JSONChildSaver;
  class JSONAttributeSaver;

  class CurrentSaveData;
  Eks::UniquePointer<CurrentSaveData> _data;
  };

/*
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
*/
}

#endif // SJSONIO_H
