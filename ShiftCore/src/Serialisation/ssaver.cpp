#include "shift/Serialisation/ssaver.h"
#include "shift/Properties/sattribute.h"

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
  return _saver->modeSymbol();
  }

const SerialisationSymbol &Saver::AttributeData::inputSymbol()
  {
  return _saver->inputSymbol();
  }

const SerialisationSymbol &Saver::AttributeData::valueSymbol()
  {
  return _saver->valueSymbol();
  }

Saver::Saver()
    : _block(nullptr)
  {
  }

Saver::WriteBlock Saver::beginWriting(QIODevice *device)
  {
  return WriteBlock(this, device);
  }

}
