#include "shift/Serialisation/ssaver.h"
#include "shift/Properties/sattribute.h"

namespace Shift
{

SaveVisitor::Visitor::SaveData::SaveData(SaveVisitor::Visitor *visitor) : _visitor(visitor)
  {
  }

Saver::WriteBlock::WriteBlock(Saver* w, QIODevice *device)
  : _oldWriteBlock(w->_block), _device(device), _written(false)
  {
  }

SaveVisitor::Visitor::AttributeData::AttributeData(SaveData *data) : _data(data)
  {
  }

Saver::WriteBlock::~WriteBlock()
  {
  _writer->_block = _oldWriteBlock;
  }

Saver::SaveDataImpl::SaveDataImpl(Attribute *root, SaveVisitor::Visitor *v)
    : SaveData(v),
      _root(root)
  {
  Saver *saver = static_cast<Saver*>(visitor());

  xAssert(saver->_block);
  xAssert(!saver->_block->_writing);
  xAssert(!saver->_block->_written);

  saver->_block->_writing = true;
  }

Saver::SaveDataImpl::~SaveDataImpl()
  {
  Saver *saver = static_cast<Saver*>(visitor());

  xAssert(saver->_block);
  xAssert(saver->_block->_writing);
  xAssert(!saver->_block->_written);

  saver->_block->_writing = false;
  saver->_block->_written = true;
  }

Saver::AttributeDataImpl::AttributeDataImpl(SaveData *data, Attribute *attr)
  : AttributeData(data), _attribute(attr)
  {
  }

Saver::Saver() : _block(nullptr)
  {
  }

Saver::WriteBlock Saver::beginWriting(QIODevice *device)
  {
  return WriteBlock(this, device);
  }


const SerialisationSymbol &Saver::AttributeDataImpl::modeSymbol()
  {
  return _saver->modeSymbol();
  }

const SerialisationSymbol &Saver::AttributeDataImpl::inputSymbol()
  {
  return _saver->inputSymbol();
  }

const SerialisationSymbol &Saver::AttributeDataImpl::valueSymbol()
  {
  return _saver->valueSymbol();
  }

}
