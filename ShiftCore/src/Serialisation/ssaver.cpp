#include "shift/Serialisation/ssaver.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/Properties/sattribute.h"
#include "shift/Properties/scontainer.h"
#include "shift/Properties/scontaineriterators.h"
#include "Memory/XTemporaryAllocator.h"

namespace Shift
{

class ChildBlock;
class ValueBlock;

//----------------------------------------------------------------------------------------------------------------------
// Saver::WriteBlock Impl
//----------------------------------------------------------------------------------------------------------------------
Saver::WriteBlock::WriteBlock(Saver* w, QIODevice *device)
  : _writer(w),
    _device(device),
    _writing(false),
    _written(false)
  {
  xAssert(!w->_block);
  w->_block = this;
  }

Saver::WriteBlock::~WriteBlock()
  {
  _writer->_block = nullptr;
  }

//----------------------------------------------------------------------------------------------------------------------
// Saver::SaveData Impl
//----------------------------------------------------------------------------------------------------------------------
Saver::Saver()
    : _block(nullptr)
  {
  }

Saver::WriteBlock Saver::beginWriting(QIODevice *device)
  {
  return WriteBlock(this, device);
  }

void Saver::begin(Eks::AllocatorBase *alloc)
  {
  xAssert(_block);
  xAssert(!_block->_writing);
  xAssert(!_block->_written);

  _block->_writing = true;

  _rootBlock = alloc->createUnique<AttributeBlock>(nullptr, Name(), alloc);
  _rootBlock->setRoot(this);
  }

void Saver::end()
  {
  _rootBlock = nullptr;

  xAssert(_block);
  xAssert(_block->_writing);
  xAssert(!_block->_written);

  _block->_writing = false;
  _block->_written = true;
  }

AttributeInterface::AttributeData *Saver::rootData()
  {
  return _rootBlock->user();
  }


void SaveBuilder::save(Attribute *attr, bool includeRoot, Saver *receiver)
  {
  Eks::TemporaryAllocator alloc(attr->temporaryAllocator());

  receiver->begin(&alloc);

  receiver->setIncludeRoot(includeRoot);

  visitAttribute(attr, receiver->rootBlock(), &alloc);

  receiver->end();
  }

void SaveBuilder::visitAttribute(Attribute *attr, Saver::AttributeBlock *data, Eks::AllocatorBase *alloc)
  {
  visitValues(attr, data, alloc);

  visitChildren(attr, data, alloc);
  }

void SaveBuilder::visitValues(Attribute *attr, Saver::AttributeBlock *data, Eks::AllocatorBase *alloc)
  {
  xAssert(attr);

  const PropertyInformation *info = attr->typeInformation();
  xAssert(info);

  const bool dyn = attr->isDynamic();

  data->saveData()->addSavedType(info, dyn);

  auto dataAttrs = data->beginValues(alloc);
  if(dyn)
    {
    dataAttrs->write(dataAttrs->typeSymbol(), info->typeName());
    }

  info->functions().save(attr, *dataAttrs.value());
  }

void SaveBuilder::visitChildren(Attribute *attr, Saver::AttributeBlock *data, Eks::AllocatorBase *attrAlloc)
  {
  Container* cont = attr->castTo<Container>();
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

        auto childData = children->addChild(child->name(), &alloc);

        if(info->functions().shouldSaveValue(child))
          {
          visitAttribute(child, childData.value(), &alloc);
          }
        }
      }
    }
  }

}
