#ifndef SATTRIBUTEINTERFACE_H
#define SATTRIBUTEINTERFACE_H

#include "Memory/XUniquePointer.h"
#include "shift/Serialisation/sattributeio.h"
#include "shift/Utilities/spropertyname.h"

namespace Shift
{

class PropertyInformation;

/// \brief The Attribute interface is an interface used to receive attribute trees.
///        Users implement functions and can the visit trees using an appropriate builder.
class AttributeInterface
  {
public:
  typedef SerialisationSymbol Symbol;

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
  class RootBlock;
  class ChildBlock;
  class ValueBlock;

  class AttributeData : public BaseData { };
  class ChildData : public BaseData { };
  class ValueData : public BaseData { };

  Eks::UniquePointer<RootBlock> begin(bool includeRoot, const PropertyInformation *info, bool dynamic, Eks::AllocatorBase *alloc);

protected:
  virtual void onBegin(AttributeData *root, bool includeRoot, Eks::AllocatorBase *alloc) = 0;
  virtual void onEnd(AttributeData *root) = 0;

  virtual void addSavedType(const PropertyInformation *info, bool dynamic) = 0;

  /// \brief symbol for attribute mode.
  virtual const SerialisationSymbol &modeSymbol() = 0;
  /// \brief symbol for input mode.
  virtual const SerialisationSymbol &inputSymbol() = 0;
  /// \brief symbol for value mode.
  virtual const SerialisationSymbol &valueSymbol() = 0;
  /// \brief symbol for type mode.
  virtual const SerialisationSymbol &typeSymbol() = 0;

  virtual Eks::UniquePointer<ChildData> onBeginChildren(AttributeData *data, ChildrenType type, Eks::AllocatorBase *alloc) = 0;
  virtual void onChildrenComplete(AttributeData *data, ChildData *) = 0;

  virtual Eks::UniquePointer<AttributeData> onAddChild(ChildData *data, const Name &name, Eks::AllocatorBase *alloc) = 0;
  virtual void onChildComplete(ChildData *, AttributeData *data) = 0;

  virtual Eks::UniquePointer<ValueData> onBeginValues(AttributeData *data, Eks::AllocatorBase *alloc) = 0;
  virtual void onValue(ValueData *, const Symbol &id, const SerialisationValue& value) = 0;
  virtual void onValuesComplete(AttributeData *data, ValueData *) = 0;
  };

template <typename T> class AttributeSubBlockHelper
  {
public:
  AttributeSubBlockHelper(AttributeInterface::AttributeBlock *owner)
      : _owner(owner)
    {
    }

  /// \brief Get the user data for the block.
  T *user() { return _user.get(); }
  /// \brief Get the owning attribute block.
  AttributeInterface::AttributeBlock *owner() { return _owner; }

protected:
  AttributeInterface::AttributeBlock *_owner;
  Eks::UniquePointer<T> _user;
  };

/// \brief An value block represents a value block for an attribute. when it is destroyed the values are complete.
/// \note It is always emitted before the children.
class AttributeInterface::ValueBlock
    : public AttributeSubBlockHelper<AttributeInterface::ValueData>
  {
public:
  ValueBlock(AttributeInterface::AttributeBlock *data, Eks::AllocatorBase *alloc);
  ~ValueBlock();

  void setValue(const Symbol &id, const SerialisationValue &value);

  /// \brief symbol for attribute mode.
  const Symbol &modeSymbol();
  /// \brief symbol for input mode.
  const Symbol &inputSymbol();
  /// \brief symbol for value mode.
  const Symbol &valueSymbol();
  };

/// \brief An child block represents the children block for an attribute. when it is destroyed the children are complete.
/// \brief Always emitted after values.
class AttributeInterface::ChildBlock : public AttributeSubBlockHelper<AttributeInterface::ChildData>
  {
public:
  ChildBlock(AttributeInterface::AttributeBlock *owner, AttributeInterface::ChildrenType type, Eks::AllocatorBase *alloc);
  ~ChildBlock();

  /// \brief Begin writing an attribute, which ends when the destructor is called.
  Eks::UniquePointer<AttributeInterface::AttributeBlock> addChild(
    const Name &name,
    const PropertyInformation *info,
    bool dynamic,
    Eks::AllocatorBase *alloc);

protected:
  /// \brief Begin writing an attribute, which ends when the destructor is called.
  void setActiveChild(AttributeInterface::AttributeBlock *a);

  /// \brief Called automatically when the Attribute Data above goes out of scope.
  void childComplete(AttributeInterface::AttributeBlock *);

private:
  AttributeInterface::AttributeBlock *_activeChild;

  friend class AttributeInterface::AttributeBlock;
  };

/// \brief An attribute block represents a visiting block for an attribute. when it is destroyed the visit is complete.
class AttributeInterface::AttributeBlock
  {
public:
  AttributeBlock(ChildBlock *parent, const Name &, const PropertyInformation *info, bool dynamic, Eks::AllocatorBase *alloc);
  ~AttributeBlock();

  /// \brief get the owning save.
  AttributeInterface* saveData() { return _data; }

  /// \brief Begin a writing child attributes.
  Eks::UniquePointer<ChildBlock> beginChildren(AttributeInterface::ChildrenType type, Eks::AllocatorBase *alloc);

  /// \brief Begin a block of values.
  Eks::UniquePointer<ValueBlock> beginValues(Eks::AllocatorBase *alloc);

  bool isRoot() const { return _parent == nullptr; }

  AttributeInterface::AttributeData *user() { return _user.get(); }

protected:
  void setRoot(AttributeInterface *data);

  void setValues(ValueBlock *vals);
  void setChildren(ChildBlock *vals);

  void childrenComplete(ChildBlock *);
  void valuesComplete(ValueBlock *);

private:
  void init(const Name &name);
  ChildBlock *_parent;
  AttributeInterface *_data;
  bool _isDynamic;
  const PropertyInformation *_type;

  ValueBlock *_values;
  ChildBlock *_children;

  Eks::UniquePointer<AttributeInterface::AttributeData> _user;

  bool _hasValues;
  bool _hasChildren;

  Eks::AllocatorBase *_alloc;

  friend class Saver;
  friend class ChildBlock;
  friend class ValueBlock;
  };

class AttributeInterface::RootBlock : public AttributeInterface::AttributeBlock
  {
public:
  RootBlock(bool includeRoot, AttributeInterface *ifc, const PropertyInformation *info, bool dynamic, Eks::AllocatorBase *alloc);

  ~RootBlock();
  };

}
#endif // SATTRIBUTEINTERFACE_H
