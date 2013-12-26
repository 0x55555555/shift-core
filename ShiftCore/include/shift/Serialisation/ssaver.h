#ifndef SSAVER_H
#define SSAVER_H

#include "shift/sglobal.h"
#include "shift/Serialisation/sattributeinterface.h"

class QIODevice;

namespace Shift
{
class PropertyInformation;

class SHIFT_EXPORT Saver : public AttributeInterface
  {
public:
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

  Saver();

  virtual void setIncludeRoot(bool include) = 0;

  AttributeBlock *rootBlock() { return _rootBlock.value(); }
  AttributeData *rootData();

  /// \brief Begin writing to [device].
  WriteBlock beginWriting(QIODevice *device);

  /// \brief Get the active write block.
  WriteBlock *activeBlock() { return _block; }

  void begin(Eks::AllocatorBase *alloc) X_OVERRIDE;
  void end() X_OVERRIDE;

private:
  WriteBlock *_block;
  Eks::UniquePointer<AttributeBlock> _rootBlock;
  friend class WriteBlock;
  };

class SHIFT_EXPORT SaveBuilder
  {
public:
  void save(Attribute *attr, bool includeRoot, Saver *receiver);

private:
  void visitAttribute(Attribute *attr, Saver::AttributeBlock *data, Eks::AllocatorBase *alloc);
  void visitValues(Attribute *attr, Saver::AttributeBlock *data, Eks::AllocatorBase *alloc);
  void visitChildren(Attribute *attr, Saver::AttributeBlock *data, Eks::AllocatorBase *alloc);
  };

}

#endif // SSAVER_H
