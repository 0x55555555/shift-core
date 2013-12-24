#ifndef SSAVER_H
#define SSAVER_H

#include "shift/sglobal.h"
#include "shift/Serialisation/sattributeio.h"
#include "Memory/XTemporaryAllocator.h"
#include "Memory/XUniquePointer.h"

class QIODevice;

namespace Shift
{
class PropertyInformation;

class SHIFT_EXPORT Saver
  {
public:
  typedef SerialisationSymbol Symbol;

  class SHIFT_EXPORT WriteBlock
    {
  XProperties:
    XROProperty(Saver *, writer);
    XROProperty(QIODevice *, device);
    XROProperty(bool, writing);
    XROProperty(bool, written);

  public:
    ~WriteBlock();

  private:
    WriteBlock(Saver *w, QIODevice *device);

    friend class Saver;
    };

  enum ChildrenType
    {
    Indexed,
    Named
    };

  class BaseData
    {
  public:
    template <typename T>T *as()
      {
      return static_cast<T *>(this);
      }
    };

  class AttributeBlock;

  class AttributeData : public BaseData { };
  class ChildData : public BaseData { };
  class ValueData : public BaseData { };

  Saver();

  virtual void setIncludeRoot(bool include) = 0;
  virtual void addSavedType(const PropertyInformation *info, bool dynamic) = 0;

  Attribute *rootAttribute() { return _rootAttribute; }
  AttributeBlock *rootBlock() { return _rootBlock.value(); }
  AttributeData *rootData();

  /// \brief Begin writing to [device].
  WriteBlock beginWriting(QIODevice *device);

  /// \brief Get the active write block.
  WriteBlock *activeBlock() { return _block; }

  /// \brief symbol for attribute mode.
  virtual const SerialisationSymbol &modeSymbol() = 0;
  /// \brief symbol for input mode.
  virtual const SerialisationSymbol &inputSymbol() = 0;
  /// \brief symbol for value mode.
  virtual const SerialisationSymbol &valueSymbol() = 0;
  /// \brief symbol for type mode.
  virtual const SerialisationSymbol &typeSymbol() = 0;

  void beginSave(Attribute *root, Eks::AllocatorBase *alloc);
  void endSave();

protected:
  virtual void onBeginSave(Attribute *root, Eks::AllocatorBase *alloc) = 0;
  virtual void onEndSave() = 0;

  virtual Eks::UniquePointer<ChildData> onBeginChildren(AttributeData *data, ChildrenType type, Eks::AllocatorBase *alloc) = 0;
  virtual void onChildrenComplete(AttributeData *data, ChildData *) = 0;

  virtual Eks::UniquePointer<AttributeData> onAddChild(ChildData *data, Attribute *a, Eks::AllocatorBase *alloc) = 0;
  virtual void onChildComplete(ChildData *, AttributeData *data) = 0;

  virtual Eks::UniquePointer<ValueData> onBeginValues(AttributeData *data, Eks::AllocatorBase *alloc) = 0;
  virtual void onWriteValue(ValueData *, const Symbol &id, const SerialisationValue& value) = 0;
  virtual void onValuesComplete(AttributeData *data, ValueData *) = 0;

private:
  WriteBlock *_block;
  Attribute *_rootAttribute;
  Eks::UniquePointer<AttributeBlock> _rootBlock;

  friend class ChildBlock;
  friend class ValueBlock;
  friend class WriteBlock;
  };

class SHIFT_EXPORT SaveBuilder
  {
public:
  void save(Attribute *attr, bool includeRoot, Saver *receiver);

private:
  void visitAttribute(Saver::AttributeBlock *data, Eks::AllocatorBase *alloc);
  void visitValues(Saver::AttributeBlock *data, Eks::AllocatorBase *alloc);
  void visitChildren(Saver::AttributeBlock *data, Eks::AllocatorBase *alloc);
  };

}

#endif // SSAVER_H
