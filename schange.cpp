#include "schange.h"
#include "sentity.h"

#if 0
STreeChange::STreeChange(const STreeEvent &ev)
    : _event(ev)
  {
  }

bool STreeChange::forward()
  {
  if(_event.oldParent())
    {
    _event.oldParent()->_children.removeAll(_event.entity());
    }

  _event.entity()->_parent = _event.newParent();
  if(_event.entity()->_parent == 0)
    {
    _event.entity()->breakConnections();
    }

  if(_event.newParent())
    {
    _event.newParent()->_children << _event.entity();
    }
  return true;
  }

bool STreeChange::backward()
  {
  if(_event.newParent())
    {
    _event.newParent()->_children.removeAll(_event.entity());
    }

  _event.entity()->_parent = _event.oldParent();
  if(_event.entity()->_parent == 0)
    {
    _event.entity()->breakConnections();
    }

  if(_event.oldParent())
    {
    _event.oldParent()->_children << _event.entity();
    }
  return true;
  }

void STreeChange::inform(Mode mode)
  {
  SEntity *changed = _event.entity();
  if(changed->parent() == 0)
    {
    if(_event.oldParent())
      {
      changed = _event.oldParent();
      }
    else if(_event.newParent())
      {
      changed = _event.newParent();
      }
    }
  xAssert(changed);
  if(mode == Forward)
    {
    Q_FOREACH(SEntity *ent, changed->parentTree())
      {
      ent->treeChanged(_event);
      }
    }
  else
    {
    STreeEvent ev(_event.type(), _event.entity(), _event.newParent(), _event.oldParent());
    if(ev.type() == STreeEvent::Create)
      {
      ev = STreeEvent(STreeEvent::Destroy, _event.entity(), _event.newParent(), _event.oldParent());
      }
    else if(ev.type() == STreeEvent::Destroy)
      {
      ev = STreeEvent(STreeEvent::Create, _event.entity(), _event.newParent(), _event.oldParent());
      }
    Q_FOREACH(SEntity *ent, changed->parentTree())
      {
      ent->treeChanged(ev);
      }
    }
  }

bool STreeChange::isValid() const
  {
  if(_event.newParent())
    {
    return _event.entity()->acceptsParent(_event.newParent()) && _event.newParent()->acceptsChild(_event.entity());
    }
  return true;
  }

SDataChange::SDataChange(const SDataEvent &ev)
    : _event(ev)
  {
  }

bool SDataChange::forward()
  {
  bool ret(_event.entity()->setData(_event.property(), _event.attribute(), _event.newValue()));

  static bool alreadyApplyingConnections = false;
  if(!alreadyApplyingConnections)
    {
    alreadyApplyingConnections = true;
    const SAttributeData &data = _event.entity()->getAttributeData(_event.property(), _event.attribute());
    ret &= _event.entity()->applyConnections(_event.property(), _event.attribute(), data.revision()+1);
    alreadyApplyingConnections = false;
    }
  return ret;
  }

bool SDataChange::backward()
  {
  bool ret(_event.entity()->setData(_event.property(), _event.attribute(), _event.oldValue()));

  static bool alreadyApplyingConnections = false;
  if(!alreadyApplyingConnections)
    {
    alreadyApplyingConnections = true;
    const SAttributeData &data = _event.entity()->getAttributeData(_event.property(), _event.attribute());
    ret &= _event.entity()->applyConnections(_event.property(), _event.attribute(), data.revision()+1);
    alreadyApplyingConnections = false;
    }
  return ret;
  }

void SDataChange::inform(Mode mode)
  {
  if(mode == Forward)
    {
    Q_FOREACH(SEntity *ent, _event.entity()->parentTree())
      {
      ent->dataChanged(_event);
      }
    }
  else
    {
    SDataEvent ev(_event.entity(), _event.property(), _event.attribute(), _event.newValue(), _event.oldValue());
    Q_FOREACH(SEntity *ent, _event.entity()->parentTree())
      {
      ent->dataChanged(ev);
      }
    }
  }

bool SDataChange::isValid() const
  {
  return true;
  }

SConnectionChange::SConnectionChange(const SConnectionEvent &ev)
    : _event(ev)
  {
  }

bool SConnectionChange::forward()
  {
  SAttributeData &outData = _event.outputEntity()->getAttributeData(_event.outputProperty(),_event.outputAttribute());
  SAttributeData &inData = _event.inputEntity()->getAttributeData(_event.inputProperty(),_event.inputAttribute());

  if(_event.type() == SConnectionEvent::Connect)
    {
    bool ret = outData.connect(inData);
    const SAttributeData &data = _event.outputEntity()->getAttributeData(_event.outputProperty(), _event.outputAttribute());
    ret &= _event.outputEntity()->applyConnections(_event.outputProperty(), _event.outputAttribute(), data.revision()+1);
    return ret;
    }
  else
    {
    return outData.disconnect(inData);
    }
  }

bool SConnectionChange::backward()
  {
  SAttributeData &outData = _event.outputEntity()->getAttributeData(_event.outputProperty(),_event.outputAttribute());
  SAttributeData &inData = _event.inputEntity()->getAttributeData(_event.inputProperty(),_event.inputAttribute());

  if(_event.type() == SConnectionEvent::Disconnect)
    {
    return outData.connect(inData);
    }
  else
    {
    return outData.disconnect(inData);
    }
  }

void SConnectionChange::inform(Mode mode)
  {
  if(mode == Forward)
    {
    _event.inputEntity()->connectionsChanged(_event);
    _event.outputEntity()->connectionsChanged(_event);
    }
  else
    {
    SConnectionEvent::Type t = SConnectionEvent::Disconnect;
    if(_event.type() == SConnectionEvent::Disconnect)
      {
      t = SConnectionEvent::Connect;
      }
    SConnectionEvent ev( t, _event.inputEntity(), _event.inputProperty(), _event.inputAttribute(),
                        _event.outputEntity(), _event.outputProperty(), _event.outputAttribute());

    _event.inputEntity()->connectionsChanged(ev);
    _event.outputEntity()->connectionsChanged(ev);
    }
  }

bool SConnectionChange::isValid() const
  {
  SAttributeData &outData = _event.outputEntity()->getAttributeData(_event.outputProperty(),_event.outputAttribute());
  SAttributeData &inData = _event.inputEntity()->getAttributeData(_event.inputProperty(),_event.inputAttribute());

  if(_event.type() == SConnectionEvent::Connect)
    {
    return outData.canConnect(inData);
    }
  else
    {
    return !outData.canConnect(inData);
    }
  }
#endif
