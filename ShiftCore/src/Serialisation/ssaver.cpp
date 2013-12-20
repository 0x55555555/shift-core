#include "shift/Serialisation/ssaver.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/Properties/sattribute.h"
#include "shift/Properties/scontainer.h"
#include "shift/Properties/scontaineriterators.h"

namespace Shift
{

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
  _writer->_saveAllocator.reset();
  }

Saver::SaveData::SaveData(Saver *v)
    : _saver(v)
  {
  xAssert(_saver->_block);
  xAssert(!_saver->_block->_writing);
  xAssert(!_saver->_block->_written);

  _saver->_block->_writing = true;
  }

Saver::SaveData::~SaveData()
  {
  xAssert(_saver->_block);
  xAssert(_saver->_block->_writing);
  xAssert(!_saver->_block->_written);

  _saver->_block->_writing = false;
  _saver->_block->_written = true;
  }

Saver::ValueData::ValueData(AttributeData *data) : _data(data)
  {
  }

const SerialisationSymbol &Saver::ValueData::modeSymbol()
  {
  return _data->saveData()->modeSymbol();
  }

const SerialisationSymbol &Saver::ValueData::inputSymbol()
  {
  return _data->saveData()->inputSymbol();
  }

const SerialisationSymbol &Saver::ValueData::valueSymbol()
  {
  return _data->saveData()->valueSymbol();
  }

const SerialisationSymbol &Saver::ValueData::typeSymbol()
  {
  return _data->saveData()->typeSymbol();
  }

Saver::AttributeData::AttributeData(SaveData *data, Attribute *attr)
    : _attribute(attr),
      _data(data)
  {
  _attributeAllocator.init(attr->temporaryAllocator());
  }

Saver::AttributeData::~AttributeData()
  {
  }

Saver::Saver()
    : _block(nullptr)
  {
  }

Saver::WriteBlock Saver::beginWriting(QIODevice *device)
  {
  return WriteBlock(this, device);
  }

void SaveVisitor::visit(Attribute *attr, bool includeRoot, Saver *receiver)
  {
  auto data = receiver->beginVisit(attr);

  data->setIncludeRoot(includeRoot);

  visitAttribute(data->rootData());
  }

void SaveVisitor::visitAttribute(Saver::AttributeData *data)
  {
  visitValues(data);

  visitChildren(data);
  }

void SaveVisitor::visitValues(Saver::AttributeData *data)
  {
  auto attr = data->attribute();
  xAssert(attr);

  const PropertyInformation *info = attr->typeInformation();
  xAssert(info);

  data->saveData()->addSavedType(info);

  auto dataAttrs = data->beginValues();
  bool dyn = attr->isDynamic();
  if(dyn)
    {
    dataAttrs->write(dataAttrs->typeSymbol(), info->typeName());
    }

  info->functions().save(attr, *dataAttrs.value());
  }

void SaveVisitor::visitChildren(Saver::AttributeData *attr)
  {
  Container* cont = attr->attribute()->castTo<Container>();
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
    auto childrenType = cont->hasNamedChildren() ? Saver::AttributeData::Named : Saver::AttributeData::Indexed;

    auto children = attr->beginChildren(childrenType);
    xAssert(children);

    xForeach(auto child, cont->walker())
      {
      const PropertyInformation *info = child->typeInformation();

      if(info->functions().shouldSave(child))
        {
        auto attr = children->beginAttribute(child);

        if(info->functions().shouldSaveValue(child))
          {
          visitAttribute(attr.value());
          }
        }
      }
    }
  }

}
