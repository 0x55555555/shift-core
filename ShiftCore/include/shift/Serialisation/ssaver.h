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

  virtual void setIncludeRoot(AttributeData *data, bool include) = 0;

  /// \brief Begin writing to [device].
  WriteBlock beginWriting(QIODevice *device);

  /// \brief Get the active write block.
  WriteBlock *activeBlock() { return _block; }

  void onBegin(AttributeData *block, Eks::AllocatorBase *alloc) X_OVERRIDE;
  void onEnd(AttributeData *block) X_OVERRIDE;

private:
  WriteBlock *_block;
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
