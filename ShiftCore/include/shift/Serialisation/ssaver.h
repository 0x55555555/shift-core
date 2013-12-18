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
    WriteBlock* _oldWriteBlock;

    friend class Saver;
    };

  class AttributeData;
  class SaveData;
  class ChildData
    {
  public:
    /// \brief Begin writing an attribute, which ends when the destructor is called.
    virtual Eks::UniquePointer<AttributeData> beginAttribute(Attribute *a) = 0;
    };

  class AttributeData : public AttributeSaver
    {
  public:
    AttributeData(SaveData *data, Attribute *attr);
    ~AttributeData();

    /// \brief get the owning save.
    SaveData* saveData() { return _data; }
    /// \brief Get the attribute being saved.
    Attribute *attribute() const { return _attribute; }
    /// \brief Get an allocator valid for the lifetime of the [AttributeData].
    Eks::TemporaryAllocator* allocator() const { return &_attributeAllocator; }

    /// \brief symbol for attribute mode.
    const Symbol &modeSymbol() X_OVERRIDE;
    /// \brief symbol for input mode.
    const Symbol &inputSymbol() X_OVERRIDE;
    /// \brief symbol for value mode.
    const Symbol &valueSymbol() X_OVERRIDE;
    /// \brief symbol for type mode.
    const Symbol &typeSymbol();

    enum ChildrenType
      {
      Indexed,
      Named
      };

    /// \brief Begin a series of calls to [beginWriting] or [writeSingleValue].
    virtual Eks::UniquePointer<ChildData> beginChildren(ChildrenType type) = 0;

  private:
    mutable Eks::TemporaryAllocator _attributeAllocator;
    Attribute *_attribute;
    SaveData *_data;

    friend class Saver;
    };

  class SaveData
    {
  public:
    SaveData(Attribute *root, Saver *visitor);
    ~SaveData();

    /// \brief Add a type which was saved to the file. Used to storve version data.
    virtual void addSavedType(const PropertyInformation *info) = 0;

    /// \brief Set if the root attribute should be saved, or if its children should be the roots.
    virtual void setIncludeRoot(bool include) = 0;

    /// \brief Get the root attributes data.
    virtual AttributeData* rootData() = 0;

    /// \brief Get the owning saver.
    Saver* saver() { return _saver; }

    /// \brief symbol for attribute mode.
    virtual const SerialisationSymbol &modeSymbol() = 0;
    /// \brief symbol for input mode.
    virtual const SerialisationSymbol &inputSymbol() = 0;
    /// \brief symbol for value mode.
    virtual const SerialisationSymbol &valueSymbol() = 0;
    /// \brief symbol for type mode.
    virtual const SerialisationSymbol &typeSymbol() = 0;

  private:
    Attribute *_root;
    Saver *_saver;
    };

  Saver();

  /// \brief Begin writing to [device].
  WriteBlock beginWriting(QIODevice *device);

  /// \brief Begin a write of a whole tree to the writer. Called once at beginning.
  virtual Eks::UniquePointer<SaveData> beginVisit(Attribute *root) = 0;

  Eks::TemporaryAllocator* saveAllocator() const { return &_saveAllocator; }

private:
  mutable Eks::TemporaryAllocator _saveAllocator;
  WriteBlock *_block;

  friend class WriteBlock;
  friend class SaveDataImpl;
  };

class SHIFT_EXPORT SaveVisitor
  {
public:
  void visit(Attribute *attr, bool includeRoot, Saver *receiver);

private:
  void visitAttribute(Saver::AttributeData *data);
  void visitChildren(Saver::AttributeData *data);
  };

}

#endif // SSAVER_H
