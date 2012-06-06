#include "sdatabasemodel.h"
#include "sentity.h"
#include "sdatabase.h"
#include "QPushButton"
#include "QStyleOptionViewItem"

Q_DECLARE_METATYPE(QModelIndex)

#define SDataModelProfileFunction XProfileFunctionBase(ShiftDataModelProfileScope)
#define SDataModelProfileScopedBlock(mess) XProfileScopedBlockBase(ShiftDataModelProfileScope, mess)

SDatabaseDelegate::SDatabaseDelegate(QObject *parent) : QItemDelegate(parent), _currentWidget(0)
  {
  }

QWidget *SDatabaseDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
  {
  if(index.isValid())
    {
    SProperty *prop = (SProperty *)index.internalPointer();
    _currentWidget = _ui.createControlWidget(prop, parent);
    if(_currentWidget)
      {
      _currentIndex = index;
      connect(_currentWidget, SIGNAL(destroyed(QObject *)), this, SLOT(currentItemDestroyed()));
      emit ((SDatabaseDelegate*)this)->sizeHintChanged(_currentIndex);
      }
    else
      {
      _currentIndex = QModelIndex();
      }
    return _currentWidget;
    }
  return 0;
  }

void SDatabaseDelegate::setEditorData(QWidget *, const QModelIndex &) const
  {
  }

void SDatabaseDelegate::setModelData(QWidget *, QAbstractItemModel *, const QModelIndex &) const
  {
  }

QSize SDatabaseDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
  {
  SDataModelProfileFunction
  if(index == _currentIndex)
    {
      xAssert(_currentWidget);
      if(_currentWidget)
      {
        return _currentWidget->sizeHint();
      }
    }
  return QItemDelegate::sizeHint(option, index);
  }

void SDatabaseDelegate::currentItemDestroyed()
  {
  emit ((SDatabaseDelegate*)this)->sizeHintChanged(_currentIndex);
  _currentIndex = QModelIndex();
  _currentWidget = 0;
  }

SDatabaseModel::SDatabaseModel(SDatabase *db, SEntity *ent, Options options) : _db(db), _root(ent),
    _options(options), _currentTreeChange(0)
  {
  if(_root == 0)
    {
    _root = db;
    }

  if(_root.isValid())
    {
    _root->addTreeObserver(this);
    }

  QHash<int, QByteArray> roles;
  roles[Qt::DisplayRole] = "name";
  roles[PropertyPositionRole] = "propertyPosition";
  roles[PropertyColourRole] = "propertyColour";
  roles[PropertyInputRole] = "propertyInput";
  roles[PropertyModeRole] = "propertyMode";
  roles[IsEntityRole] = "isEntity";
  roles[EntityInputPositionRole] = "entityInputPosition";
  roles[EntityOutputPositionRole] = "entityOutputPosition";
  setRoleNames(roles);
  }

SDatabaseModel::~SDatabaseModel()
  {
  if(_root.isValid())
    {
    _root->removeTreeObserver(this);
    }
  }

QModelIndex SDatabaseModel::index(const SProperty *p) const
  {
  return createIndex(p->index(), 0, (SProperty *)p);
  }

bool SDatabaseModel::isEqual(const QModelIndex &a, const QModelIndex &b) const
  {
  const void *ap = a.internalPointer();
  const void *bp = b.internalPointer();
  if(!ap)
    {
    ap = _root.entity();
    }
  if(!bp)
    {
    bp = _root.entity();
    }
  return ap == bp;
  }

QModelIndex SDatabaseModel::root() const
  {
  return createIndex(0, 0, (SProperty*)_root.entity());
  }

bool SDatabaseModel::isValid(const QModelIndex &a) const
  {
  return a.isValid();
  }

int SDatabaseModel::rowIndex(const QModelIndex &i) const
  {
  return i.row();
  }

int SDatabaseModel::columnIndex(const QModelIndex &i) const
  {
  return i.column();
  }

int SDatabaseModel::rowCount( const QModelIndex &parent ) const
  {
  SDataModelProfileFunction
  const SProperty *prop = _root;
  if(parent.isValid())
    {
    prop = (SProperty *)parent.internalPointer();
    }

  if(!prop)
    {
    return 0;
    }

  if(_options.hasFlag(EntitiesOnly))
    {
    const SEntity *ent = prop->castTo<SEntity>();
    xAssert(ent);

    prop = &ent->children;
    }

  const SPropertyContainer *container = prop->castTo<SPropertyContainer>();
  if(container)
    {
    if(_currentTreeChange)
      {
      xAssert(container != _currentTreeChange->property());
      if(container == _currentTreeChange->after())
        {
        return container->size() - 1;
        }
      else if(container == _currentTreeChange->before())
        {
        return container->size() + 1;
        }
      }
    return container->size();
    }

  return 0;
  }

QModelIndex SDatabaseModel::index( int row, int column, const QModelIndex &parent ) const
  {
  SDataModelProfileFunction
  const SProperty *prop = _root;
  int size = 0;
  if(parent.isValid())
    {
    prop = (SProperty *)parent.internalPointer();
    }

  if(!prop)
    {
    return QModelIndex();
    }

  if(_options.hasFlag(EntitiesOnly))
    {
    const SEntity *ent = prop->castTo<SEntity>();
    xAssert(ent);

    prop = &ent->children;
    }

  const SPropertyContainer *container = prop->castTo<SPropertyContainer>();
  if(container)
    {
    if(_currentTreeChange)
      {
      xAssert(container != _currentTreeChange->property());
      if(container == _currentTreeChange->before())
        {
        xsize oldRow = xMin(container->size(), _currentTreeChange->index());
        if((xsize)row == oldRow)
          {
          return createIndex(row, column, _currentTreeChange->property());
          }
        else if((xsize)row > oldRow)
          {
          --row;
          }
        }
      else if(container == _currentTreeChange->after())
        {
        xsize newRow = xMin(container->size()-1, _currentTreeChange->index());
        if((xsize)row >= newRow)
          {
          ++row;
          }
        }
      }
    SProperty *child = container->firstChild();
    while(child)
      {
      if(size == row)
        {
        return createIndex(row, column, child);
        }
      size++;
      child = child->nextSibling();
      }
    xAssertFail();
    }

  return QModelIndex();
  }

QModelIndex SDatabaseModel::parent( const QModelIndex &child ) const
  {
  SDataModelProfileFunction
  if(child.isValid())
    {
    SProperty *prop = (SProperty *)child.internalPointer();
    SPropertyContainer *parent = prop->parent();

    if(_currentTreeChange)
      {
      if(prop == _currentTreeChange->property())
        {
        parent = (SPropertyContainer*)_currentTreeChange->before();
        }
      }

    if(parent)
      {
      if(_options.hasFlag(EntitiesOnly))
        {
        return createIndex(parent->entity()->index(), 0, (SProperty*)parent->entity());
        }
      else
        {
        return createIndex(parent->index(), 0, (SProperty*)parent);
        }
      }
    }
  return QModelIndex();
  }

int SDatabaseModel::columnCount( const QModelIndex &parent ) const
  {
  SDataModelProfileFunction
  const SProperty *prop = _root;
  if(parent.isValid())
    {
    prop = (SProperty *)parent.internalPointer();
    }

  if(_options.hasFlag(ShowValues) && prop)
    {
    xsize columns = 1;

    const SPropertyContainer *cont = prop->castTo<SPropertyContainer>();
    if(cont)
      {
      SProperty *child = cont->firstChild();
      while(child)
        {
        // this could maybe be improved, but we dont want to show the values for complex widgets...
        SPropertyVariantInterface *interface = child->interface<SPropertyVariantInterface>();
        if(interface)
          {
          columns = 2;
          break;
          }

        child = child->nextSibling();
        }
      }
    return columns;
    }
  return 1;
  }

QVariant SDatabaseModel::data( const QModelIndex &index, int role ) const
  {
  SDataModelProfileFunction
  const SProperty *prop = (const SProperty *)index.internalPointer();
  if(!index.isValid())
    {
    if(role == PropertyColourRole)
      {
      const SPropertyColourInterface *interface = _root->interface<SPropertyColourInterface>();
      if(interface)
        {
        return interface->colour(_root).toLDRColour();
        }
      return QColor();
      }
    return QVariant();
    }
  xAssert(prop);

  xAssert(!_currentTreeChange || _currentTreeChange->property() != prop);

  if(role == Qt::DisplayRole)
    {
    if(_options.hasFlag(ShowValues) && index.column() == 1)
      {
      const SPropertyVariantInterface *interface = prop->interface<SPropertyVariantInterface>();
      if(interface)
        {
        return interface->asVariant(prop);
        }
      return QVariant();
      }
    else
      {
      QString name = prop->name();
      xAssert(!name.isEmpty());
      return name;
      }
    }
  else if(role == PropertyPositionRole)
    {
    const SPropertyPositionInterface *interface = prop->interface<SPropertyPositionInterface>();
    if(interface)
      {
      return toQt(interface->position(prop));
      }
    return QVector3D();
    }
  else if(role == EntityInputPositionRole)
    {
    const SPropertyPositionInterface *interface = prop->interface<SPropertyPositionInterface>();
    if(interface)
      {
      return toQt(interface->inputsPosition(prop));
      }
    return QVector3D();
    }
  else if(role == EntityOutputPositionRole)
    {
    const SPropertyPositionInterface *interface = prop->interface<SPropertyPositionInterface>();
    if(interface)
      {
      return toQt(interface->outputsPosition(prop));
      }
    return QVector3D();
    }
  else if(role == PropertyColourRole)
    {
    const SPropertyColourInterface *interface = prop->interface<SPropertyColourInterface>();
    if(interface)
      {
      return interface->colour(prop).toLDRColour();
      }
    return QColor();
    }
  else if(role == PropertyInputRole)
    {
    SProperty *inp = prop->input();
    if(inp)
      {
      return QVariant::fromValue(createIndex(inp->index(), 0, inp));
      }
    else
      {
      return QVariant::fromValue(QModelIndex());
      }
    }
  else if(role == PropertyModeRole)
    {
    const SPropertyInstanceInformation *inst = prop->instanceInformation();
    xAssert(inst);

    return inst->modeString();
    }
  else if(role == IsEntityRole)
    {
    return prop->entity() == prop;
    }

  return QVariant();
  }

QVariant SDatabaseModel::data( const QModelIndex &index, const QString &role) const
  {
  SDataModelProfileFunction
  const QHash<int, QByteArray> &roles = roleNames();

  QHash<int, QByteArray>::const_iterator it = roles.begin();
  QHash<int, QByteArray>::const_iterator end = roles.end();
  for(; it != end; ++it)
    {
    const QByteArray &name = it.value();

    if(role == name)
      {
      return data(index, it.key());
      }
    }

  return QVariant();
  }

bool SDatabaseModel::setData(const QModelIndex &index, const QVariant &val, int role)
  {
  xAssert(!_currentTreeChange);
  SDataModelProfileFunction
  SProperty *prop = (SProperty *)index.internalPointer();
  if(prop)
    {
    if(role == Qt::DisplayRole)
      {
      if(_options.hasFlag(ShowValues) && index.column() == 1)
        {
        SPropertyVariantInterface *interface = prop->interface<SPropertyVariantInterface>();
        if(interface)
          {
          interface->setVariant(prop, val);
          return true;
          }
        }
      else
        {
        prop->setName(val.toString());
        return true;
        }
      }
    else if(role == PropertyPositionRole)
      {
      SPropertyPositionInterface *interface = prop->interface<SPropertyPositionInterface>();
      if(interface)
        {
        QVector3D vec = val.value<QVector3D>();
        interface->setPosition(prop, XVector3D(vec.x(), vec.y(), vec.z()));
        return true;
        }
      }
    else if(role == EntityInputPositionRole)
      {
      SPropertyPositionInterface *interface = prop->interface<SPropertyPositionInterface>();
      if(interface)
        {
        QVector3D vec = val.value<QVector3D>();
        interface->setInputsPosition(prop, XVector3D(vec.x(), vec.y(), vec.z()));
        return true;
        }
      }
    else if(role == EntityOutputPositionRole)
      {
      SPropertyPositionInterface *interface = prop->interface<SPropertyPositionInterface>();
      if(interface)
        {
        QVector3D vec = val.value<QVector3D>();
        interface->setOutputsPosition(prop, XVector3D(vec.x(), vec.y(), vec.z()));
        return true;
        }
      }
    else if(role == PropertyInputRole)
      {
      QModelIndex inputIndex = val.value<QModelIndex>();
      SProperty *input = (SProperty *)inputIndex.internalPointer();

      input->connect(prop);
      }
    }
  return false;
  }

bool SDatabaseModel::setData(const QModelIndex &index, const QString &role, const QVariant &value)
  {
  SDataModelProfileFunction
  const QHash<int, QByteArray> &roles = roleNames();

  QHash<int, QByteArray>::const_iterator it = roles.begin();
  QHash<int, QByteArray>::const_iterator end = roles.end();
  for(; it != end; ++it)
    {
    const QByteArray &name = it.value();

    if(role == name)
      {
      return setData(index, value, it.key());
      }
    }

  return false;
  }

QVariant SDatabaseModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
    if(section == 0)
      {
      return "Property";
      }
    else if(section == 1)
      {
      return "Value";
      }
    }
  return QVariant();
  }

Qt::ItemFlags SDatabaseModel::flags(const QModelIndex &index) const
  {
  SDataModelProfileFunction
  SProperty *prop = (SProperty *)index.internalPointer();
  xAssert(!_currentTreeChange || _currentTreeChange->property() != prop);
  if(prop && index.column() < 2)
    {
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }
  return QAbstractItemModel::flags(index);
  }

void SDatabaseModel::onTreeChange(const SChange *c, bool back)
  {
  const SEntity::TreeChange *tC = c->castTo<SEntity::TreeChange>();
  if(tC)
    {
    xAssert(!_currentTreeChange);
    _currentTreeChange = tC;

    if(tC->property() == _root && tC->after(back) == 0)
      {
      _root = 0;
      }

    emit layoutAboutToBeChanged();

    if(tC->after(back) == 0)
      {
      changePersistentIndex(createIndex(tC->index(), 0, tC->property()), QModelIndex());

      const SPropertyContainer *parent = tC->before(back);
      xAssert(parent);

      xsize i = tC->index();
      emit beginRemoveRows(createIndex(parent->index(), 0, (SProperty*)parent), i, i);
      _currentTreeChange = 0;
      emit endRemoveRows();
      }
    else
      {
      const SPropertyContainer *parent = tC->after(back);
      xAssert(parent);

      xsize i = xMin(parent->size()-1, tC->index());
      emit beginInsertRows(createIndex(parent->index(), 0, (SProperty*)parent), i, i);
      _currentTreeChange = 0;
      emit endInsertRows();
      }

    emit layoutChanged();
    }

  const SProperty::NameChange *nameChange = c->castTo<SProperty::NameChange>();
  if(nameChange)
    {
    const SProperty *prop = nameChange->property();
    QModelIndex ind = createIndex(prop->index(), 0, (SProperty*)prop);
    emit dataChanged(ind, ind);
    }
  }

void SDatabaseModel::actOnChanges()
  {
  }

void SDatabaseModel::setOptions(Options options)
  {
  emit layoutAboutToBeChanged();
  _options = options;
  emit layoutChanged();
  }

SDatabaseModel::Options SDatabaseModel::options() const
  {
  return _options;
  }

void SDatabaseModel::setRoot(SEntity *ent)
  {
  beginResetModel();
  if(_root)
    {
    _root->removeTreeObserver(this);
    }
  _root = ent;

  if(_root)
    {
    _root->addTreeObserver(this);
    }
  endResetModel();

  emit dataChanged(index(0, 0), index(_root->children.size(), 0));
  }

void SDatabaseModel::setDatabase(SDatabase *db, SEntity *root)
  {
  _db = db;

  if(root == 0)
    {
    root = db;
    }

  setRoot(root);
  }
