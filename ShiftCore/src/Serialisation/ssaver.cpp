#include "shift/Serialisation/ssaver.h"
#include "shift/TypeInformation/spropertyinformation.h"
#include "shift/Properties/sattribute.h"
#include "shift/Properties/scontainer.h"
#include "shift/Properties/scontaineriterators.h"

namespace Shift
{

Saver::WriteBlock::WriteBlock(Saver* w, QIODevice *device)
  : _oldWriteBlock(w->_block),
    _device(device),
    _written(false)
  {
  }

Saver::WriteBlock::~WriteBlock()
  {
  _writer->_block = _oldWriteBlock;
  }

Saver::SaveData::SaveData(Attribute *root, Saver *v)
    : _root(root),
      _saver(v)
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

Saver::AttributeData::AttributeData(SaveData *data, Attribute *attr)
    : _data(data),
      _attribute(attr)
  {
  }

Saver::AttributeData::~AttributeData()
  {
  }

const SerialisationSymbol &Saver::AttributeData::modeSymbol()
  {
  return _data->modeSymbol();
  }

const SerialisationSymbol &Saver::AttributeData::inputSymbol()
  {
  return _data->inputSymbol();
  }

const SerialisationSymbol &Saver::AttributeData::valueSymbol()
  {
  return _data->valueSymbol();
  }

const SerialisationSymbol &Saver::AttributeData::typeSymbol()
  {
  return _data->typeSymbol();
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
  auto attr = data->attribute();
  xAssert(attr);

  const PropertyInformation *info = attr->typeInformation();
  xAssert(info);

  data->saveData()->addSavedType(info);

  bool dyn = attr->isDynamic();
  if(dyn)
    {
    data->write(data->typeSymbol(), info->typeName());
    }

  info->functions().save(attr, *data);

  visitChildren(data);
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
