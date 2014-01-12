#include "shift/Serialisation/ssaver.h"
#include "shift/Properties/sattribute.h"
#include "shift/Properties/scontainer.h"
#include "shift/Properties/scontaineriterators.h"
#include "shift/Properties/sattribute.inl"
#include "shift/Properties/scontainer.inl"
#include "Memory/XTemporaryAllocator.h"

namespace Shift
{

class ChildBlock;
class ValueBlock;

//----------------------------------------------------------------------------------------------------------------------
// Saver::WriteBlock Impl
//----------------------------------------------------------------------------------------------------------------------
Saver::WriteBlock::WriteBlock(Saver* w, QIODevice *device)
    : IOBlock(w),
      _device(device)
  {
  }

//----------------------------------------------------------------------------------------------------------------------
// Saver::SaveData Impl
//----------------------------------------------------------------------------------------------------------------------
Saver::Saver()
  {
  }

Eks::UniquePointer<Saver::WriteBlock> Saver::beginWriting(QIODevice *device)
  {
  return Eks::Core::globalAllocator()->createUnique<WriteBlock>(this, device);
  }

QIODevice *Saver::activeDevice()
  {
  return static_cast<Saver::WriteBlock*>(activeBlock())->device();
  }

void Saver::onBegin(AttributeData *, bool, Eks::AllocatorBase *)
  {
  IOBlockUser::begin();
  }

void Saver::onEnd(AttributeData *)
  {
  IOBlockUser::end();
  }

void SaveBuilder::save(const Attribute *attr, bool includeRoot, AttributeInterface *receiver)
  {
  Eks::TemporaryAllocator alloc(attr->temporaryAllocator());

  auto block = receiver->begin(includeRoot, attr->typeInformation(), attr->isDynamic(), &alloc);

  visitAttribute(!includeRoot, attr, block.value(), &alloc);
  }

void SaveBuilder::visitAttribute(bool topLevel, const Attribute *attr, Saver::AttributeBlock *data, Eks::AllocatorBase *alloc)
  {
  visitValues(attr, data, alloc);

  visitChildren(topLevel, attr, data, alloc);
  }

void SaveBuilder::visitValues(const Attribute *attr, Saver::AttributeBlock *data, Eks::AllocatorBase *alloc)
  {
  xAssert(attr);

  const PropertyInformation *info = attr->typeInformation();
  xAssert(info);

  auto dataAttrs = data->beginValues(alloc);

  struct SaveHelper : public AttributeSaver
    {
    const Symbol &modeSymbol() const X_OVERRIDE { return block->modeSymbol(); }
    const Symbol &valueSymbol() const X_OVERRIDE { return block->valueSymbol(); }
    const Symbol &inputSymbol() const X_OVERRIDE { return block->inputSymbol(); }

    void writeValue(const Symbol &id, const SerialisationValue& value) X_OVERRIDE
      {
      block->setValue(id, value);
      }

    Saver::ValueBlock *block;
    } helper;

  helper.block = dataAttrs.value();

  info->functions().save(attr, helper);
  }

void SaveBuilder::visitChildren(bool topLevel, const Attribute *attr, Saver::AttributeBlock *data, Eks::AllocatorBase *attrAlloc)
  {
  const Container* cont = attr->castTo<Container>();
  if (!cont)
    {
    return;
    }

  bool shouldSaveAnyChildren = false;
  xForeach(auto child, cont->walker())
    {
    const PropertyInformation *info = child->typeInformation();

    if(info->functions().shouldSave(child))
      {
      shouldSaveAnyChildren = true;
      break;
      }
    }

  if(shouldSaveAnyChildren)
    {
    auto childrenType = cont->hasNamedChildren() ? Saver::Named : Saver::Indexed;

    auto children = data->beginChildren(childrenType, attrAlloc);
    xAssert(children);

    xForeach(auto child, cont->walker())
      {
      const PropertyInformation *info = child->typeInformation();

      if(info->functions().shouldSave(child))
        {
        Eks::TemporaryAllocator alloc(child->temporaryAllocator());

        auto childData = children->addChild(child->name(), child->typeInformation(), topLevel || child->isDynamic(), &alloc);

        visitAttribute(false, child, childData.value(), &alloc);
        }
      }
    }
  }

}
