#ifndef SJSONIO_H
#define SJSONIO_H

#include "QBuffer"
#include "sloader.h"
#include "shift/Serialisation/ssaver.h"
#include "shift/Serialisation/sattributeinterface.h"
#include "QXmlStreamWriter"
#include "QXmlStreamReader"
#include "Memory/XUniquePointer.h"
#include "Memory/XTemporaryAllocator.h"
#include "Containers/XUnorderedMap.h"

namespace Shift
{

class SHIFT_EXPORT JSONSaver : public Saver
  {
XProperties:
  XProperty(bool, autoWhitespace, setAutoWhitespace);

public:
  JSONSaver();

private:
  void emitJson(Eks::AllocatorBase *alloc, AttributeData *root, QIODevice *dev);
  void addSavedType(const PropertyInformation *info, bool dynamic) X_OVERRIDE;
  const SerialisationSymbol &modeSymbol() X_OVERRIDE;
  const SerialisationSymbol &inputSymbol() X_OVERRIDE;
  const SerialisationSymbol &valueSymbol() X_OVERRIDE;
  const SerialisationSymbol &typeSymbol() X_OVERRIDE;
  const SerialisationSymbol &childrenSymbol();
  void onBegin(AttributeData *block, bool includeRoot, Eks::AllocatorBase *alloc) X_OVERRIDE;
  void onEnd(AttributeData *block) X_OVERRIDE;
  Eks::UniquePointer<Saver::ChildData> onBeginChildren(AttributeData *data, Saver::ChildrenType type, Eks::AllocatorBase *alloc) X_OVERRIDE;
  void onChildrenComplete(AttributeData *data, ChildData *) X_OVERRIDE;
  Eks::UniquePointer<Saver::AttributeData> onAddChild(Saver::ChildData *parent, const Shift::Name &name, Eks::AllocatorBase *alloc) X_OVERRIDE;
  void onChildComplete(Saver::ChildData *, AttributeData *child) X_OVERRIDE;
  void completeAttribute(AttributeData *data);
  Eks::UniquePointer<Saver::ValueData> onBeginValues(AttributeData *, Eks::AllocatorBase *allocator) X_OVERRIDE;
  void onValuesComplete(AttributeData *data, ValueData *v) X_OVERRIDE;
  void onValue(Saver::ValueData *v, const Symbol &id, const SerialisationValue& value) X_OVERRIDE;

  class JSONValueSaver;
  class JSONChildSaver;
  class JSONAttributeSaver;

  class CurrentSaveData;
  Eks::UniquePointer<CurrentSaveData> _data;
  };

class SHIFT_EXPORT JSONLoader
  {
public:
  JSONLoader();
  ~JSONLoader();

  void load(QIODevice *device, AttributeInterface *parent);
  };

}

#endif // SJSONIO_H
