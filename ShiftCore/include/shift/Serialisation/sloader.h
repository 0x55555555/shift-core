#ifndef SLOADER_H
#define SLOADER_H

#include "shift/sglobal.h"
#include "shift/Utilities/spropertyname.h"
#include "shift/Serialisation/sioblock.h"
#include "shift/Serialisation/sattributeinterface.h"
#include "Utilities/XProperty.h"
#include "Containers/XStringSimple.h"
#include "Memory/XTemporaryAllocator.h"
#include "QtCore/QTextStream"
#include "QtCore/QDataStream"

class QString;

namespace Shift
{

class Property;
class Container;
class PropertyInformation;

class SHIFT_EXPORT LoadBuilder : public AttributeInterface, public IOBlockUser
  {
public:
  class LoadBlock : private IOBlock
    {
  XProperties:
    XROProperty(Attribute *, root);
    XROByRefProperty(Eks::Vector<Attribute*>, loadedData);

  public:
    LoadBlock(LoadBuilder *w, Attribute *root, Eks::AllocatorBase *alloc);

  private:
    friend class LoadBuilder;
    };

  LoadBuilder();

  /// \brief Begin loading under to [device].
  Eks::UniquePointer<LoadBlock> beginLoading(Attribute *root, Eks::AllocatorBase *);

protected:
  void onBegin(AttributeData *root, bool includeRoot, Eks::AllocatorBase *alloc) X_OVERRIDE;
  void onEnd(AttributeData *root) X_OVERRIDE;

  void addSavedType(const PropertyInformation *info, bool dynamic) X_OVERRIDE;

  const SerialisationSymbol &modeSymbol() X_OVERRIDE;
  const SerialisationSymbol &inputSymbol() X_OVERRIDE;
  const SerialisationSymbol &valueSymbol() X_OVERRIDE;
  const SerialisationSymbol &typeSymbol() X_OVERRIDE;

  Eks::UniquePointer<ChildData> onBeginChildren(AttributeData *data, ChildrenType type, Eks::AllocatorBase *alloc) X_OVERRIDE;
  void onChildrenComplete(AttributeData *data, ChildData *) X_OVERRIDE;

  Eks::UniquePointer<AttributeData> onAddChild(ChildData *data, const Name &name, Eks::AllocatorBase *alloc) X_OVERRIDE;
  void onChildComplete(ChildData *, AttributeData *data) X_OVERRIDE;

  Eks::UniquePointer<ValueData> onBeginValues(AttributeData *data, Eks::AllocatorBase *alloc) X_OVERRIDE;
  void onValue(ValueData *, const Symbol &id, const SerialisationValue& value) X_OVERRIDE;
  void onValuesComplete(AttributeData *data, ValueData *) X_OVERRIDE;

private:
  class LoadData;
  Eks::UniquePointer<LoadData> _currentData;
  };

/*
class Loader
  {
public:
  enum Mode
    {
    Text,
    Binary
    };

XProperties:
  XROProperty(Mode, streamMode);

public:
  virtual ~Loader() { }

  void setStreamDevice(Mode m, QIODevice *d) { _streamMode = m; _ts.setDevice(d); _ds.setDevice(d); }

  struct CurrentData
    {
    Attribute *existing;
    const PropertyInformation *type;
    Name name;
    bool dynamic;
    };

  virtual const CurrentData *currentData() const { return &_data; }

  void loadChildren(Container *parent);

  virtual bool beginChildren() const X_OVERRIDE;
  virtual void endChildren() const X_OVERRIDE;
  virtual bool hasNextChild() const X_OVERRIDE;

  virtual void beginNextChild() X_OVERRIDE;
  virtual bool childHasValue() const X_OVERRIDE;
  virtual void endNextChild() X_OVERRIDE;

  void read(Container *parent);

  virtual void beginAttribute(const char *) X_OVERRIDE;
  virtual void endAttribute(const char *) X_OVERRIDE;

  typedef Eks::StringBase<Eks::Char, 1024> InputString;
  virtual void resolveInputAfterLoad(Property *, const InputString &) X_OVERRIDE;

  QTextStream &textStream() { return _ts; }
  QDataStream &binaryStream() { return _ds; }

private:
  QTextStream _ts;
  QDataStream _ds;

  CurrentData _data;
  };*/


}

#endif // SLOADER_H
