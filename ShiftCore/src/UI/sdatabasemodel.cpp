#include "shift/UI/sdatabasemodel.h"
#include "shift/Properties/spropertycontaineriterators.h"
#include "shift/TypeInformation/sinterfaces.h"
#include "shift/sentity.h"
#include "shift/sdatabase.h"
#include "QtWidgets/QPushButton"
#include "QtWidgets/QStyleOptionViewItem"

namespace Shift
{

Q_DECLARE_METATYPE(QModelIndex)

#define SDataModelProfileFunction XProfileFunctionBase(ShiftDataModelProfileScope)
#define SDataModelProfileScopedBlock(mess) XProfileScopedBlockBase(ShiftDataModelProfileScope, mess)

DatabaseDelegate::DatabaseDelegate(QObject *parent) : QStyledItemDelegate(parent), _currentWidget(0)
  {
  }

QWidget *DatabaseDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
  {
  if(index.isValid())
    {
    Property *prop = (Property *)index.internalPointer();
    _currentWidget = _ui.createControlWidget(prop, parent);
    if(_currentWidget)
      {
      _currentIndex = index;
      connect(_currentWidget, SIGNAL(destroyed(QObject *)), this, SLOT(currentItemDestroyed()));
      Q_EMIT ((DatabaseDelegate*)this)->sizeHintChanged(_currentIndex);
      }
    else
      {
      _currentIndex = QModelIndex();
      }
    return _currentWidget;
    }
  return 0;
  }

void DatabaseDelegate::setEditorData(QWidget *, const QModelIndex &) const
  {
  }

void DatabaseDelegate::setModelData(QWidget *, QAbstractItemModel *, const QModelIndex &) const
  {
  }

QSize DatabaseDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
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
  return QStyledItemDelegate::sizeHint(option, index);
  }

void DatabaseDelegate::currentItemDestroyed()
  {
  Q_EMIT ((DatabaseDelegate*)this)->sizeHintChanged(_currentIndex);
  _currentIndex = QModelIndex();
  _currentWidget = 0;
  }

DatabaseModel::DatabaseModel(Database *db, Entity *ent, Options options) : _db(db), _root(ent),
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

  _roles[Qt::DisplayRole] = "name";
  _roles[PropertyPositionRole] = "propertyPosition";
  _roles[PropertyColourRole] = "propertyColour";
  _roles[PropertyInputRole] = "propertyInput";
  _roles[PropertyModeRole] = "propertyMode";
  _roles[IsEntityRole] = "isEntity";
  _roles[EntityInputPositionRole] = "entityInputPosition";
  _roles[EntityOutputPositionRole] = "entityOutputPosition";
  }

DatabaseModel::~DatabaseModel()
  {
  if(_root.isValid())
    {
    _root->removeTreeObserver(this);
    }
  }

QModelIndex DatabaseModel::index(const Property *p) const
  {
  return createIndex(p->parent()->index(p), 0, (Property *)p);
  }

bool DatabaseModel::isEqual(const QModelIndex &a, const QModelIndex &b) const
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

QModelIndex DatabaseModel::root() const
  {
  return createIndex(0, 0, (Property*)_root.entity());
  }

bool DatabaseModel::isValid(const QModelIndex &a) const
  {
  return a.isValid();
  }

int DatabaseModel::rowIndex(const QModelIndex &i) const
  {
  return i.row();
  }

int DatabaseModel::columnIndex(const QModelIndex &i) const
  {
  return i.column();
  }

int DatabaseModel::rowCount( const QModelIndex &parent ) const
  {
  SDataModelProfileFunction
  const Property *prop = _root;
  if(parent.isValid())
    {
    prop = (Property *)parent.internalPointer();
    }

  if(!prop)
    {
    return 0;
    }

  if(_options.hasFlag(EntitiesOnly))
    {
    const Entity *ent = prop->castTo<Entity>();
    xAssert(ent);

    prop = &ent->children;
    }

  const PropertyContainer *container = prop->castTo<PropertyContainer>();
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

QModelIndex DatabaseModel::index( int row, int column, const QModelIndex &parent ) const
  {
  SDataModelProfileFunction
  const Property *prop = _root;
  int size = 0;
  if(parent.isValid())
    {
    prop = (Property *)parent.internalPointer();
    }

  if(!prop)
    {
    return QModelIndex();
    }

  if(_options.hasFlag(EntitiesOnly))
    {
    const Entity *ent = prop->castTo<Entity>();
    xAssert(ent);

    prop = &ent->children;
    }

  const PropertyContainer *container = prop->castTo<PropertyContainer>();
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

    xForeach(auto child, container->walker())
      {
      if(size == row)
        {
        return createIndex(row, column, (void*)child);
        }
      size++;
      }
    xAssertFail();
    }

  return QModelIndex();
  }

QModelIndex DatabaseModel::parent( const QModelIndex &child ) const
  {
  SDataModelProfileFunction
  if(child.isValid())
    {
    Property *prop = (Property *)child.internalPointer();
    PropertyContainer *parent = prop->parent();

    if(_currentTreeChange)
      {
      if(prop == _currentTreeChange->property())
        {
        parent = (PropertyContainer*)_currentTreeChange->before();
        }
      }

    if(parent)
      {
      if(_options.hasFlag(EntitiesOnly))
        {
        Entity *ent = parent->entity();
        return createIndex(ent->parent()->index(ent), 0, (Property*)ent);
        }
      else
        {
        return createIndex(parent->parent()->index(parent), 0, (Property*)parent);
        }
      }
    }
  return QModelIndex();
  }

int DatabaseModel::columnCount( const QModelIndex &parent ) const
  {
  SDataModelProfileFunction
  const Property *prop = _root;
  if(parent.isValid())
    {
    prop = (Property *)parent.internalPointer();
    }

  if(_options.hasFlag(ShowValues) && prop)
    {
    xsize columns = 1;

    const PropertyContainer *cont = prop->castTo<PropertyContainer>();
    if(cont)
      {
      xForeach(auto child, cont->walker())
        {
        // this could maybe be improved, but we dont want to show the values for complex widgets...
        const PropertyVariantInterface *interface = child->interface<PropertyVariantInterface>();
        if(interface)
          {
          columns = 2;
          break;
          }
        }
      }
    return columns;
    }
  return 1;
  }

QVariant DatabaseModel::data( const QModelIndex &index, int role ) const
  {
  SDataModelProfileFunction
  const Property *prop = (const Property *)index.internalPointer();
  if(!index.isValid())
    {
    if(role == PropertyColourRole)
      {
      const PropertyColourInterface *interface = _root->interface<PropertyColourInterface>();
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
      const PropertyVariantInterface *interface = prop->interface<PropertyVariantInterface>();
      if(interface)
        {
        return interface->asVariant(prop);
        }
      return QVariant();
      }
    else
      {
      QString name = prop->name().toQString();
      xAssert(!name.isEmpty());
      return name;
      }
    }
  else if(role == PropertyPositionRole)
    {
    const PropertyPositionInterface *interface = prop->interface<PropertyPositionInterface>();
    if(interface)
      {
      return toQt(interface->position(prop));
      }
    return QVector3D();
    }
  else if(role == EntityInputPositionRole)
    {
    const PropertyPositionInterface *interface = prop->interface<PropertyPositionInterface>();
    if(interface)
      {
      return toQt(interface->inputsPosition(prop));
      }
    return QVector3D();
    }
  else if(role == EntityOutputPositionRole)
    {
    const PropertyPositionInterface *interface = prop->interface<PropertyPositionInterface>();
    if(interface)
      {
      return toQt(interface->outputsPosition(prop));
      }
    return QVector3D();
    }
  else if(role == PropertyColourRole)
    {
    const PropertyColourInterface *interface = prop->interface<PropertyColourInterface>();
    if(interface)
      {
      return interface->colour(prop).toLDRColour();
      }
    return QColor();
    }
  else if(role == PropertyInputRole)
    {
    Property *inp = prop->input();
    if(inp)
      {
      return QVariant::fromValue(createIndex(inp->parent()->index(inp), 0, inp));
      }
    else
      {
      return QVariant::fromValue(QModelIndex());
      }
    }
  else if(role == PropertyModeRole)
    {
    const PropertyInstanceInformation *inst = prop->baseInstanceInformation();
    xAssert(inst);

    return inst->modeString().toQString();
    }
  else if(role == IsEntityRole)
    {
    return prop->entity() == prop;
    }

  return QVariant();
  }

QVariant DatabaseModel::data( const QModelIndex &index, const QString &role) const
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

bool DatabaseModel::setData(const QModelIndex &index, const QVariant &val, int role)
  {
  xAssert(!_currentTreeChange);
  SDataModelProfileFunction
  Property *prop = (Property *)index.internalPointer();
  if(prop)
    {
    if(role == Qt::DisplayRole)
      {
      if(_options.hasFlag(ShowValues) && index.column() == 1)
        {
        PropertyVariantInterface *interface = prop->interface<PropertyVariantInterface>();
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
      PropertyPositionInterface *interface = prop->interface<PropertyPositionInterface>();
      if(interface)
        {
        QVector3D vec = val.value<QVector3D>();
        interface->setPosition(prop, Eks::Vector3D(vec.x(), vec.y(), vec.z()));
        return true;
        }
      }
    else if(role == EntityInputPositionRole)
      {
      PropertyPositionInterface *interface = prop->interface<PropertyPositionInterface>();
      if(interface)
        {
        QVector3D vec = val.value<QVector3D>();
        interface->setInputsPosition(prop, Eks::Vector3D(vec.x(), vec.y(), vec.z()));
        return true;
        }
      }
    else if(role == EntityOutputPositionRole)
      {
      PropertyPositionInterface *interface = prop->interface<PropertyPositionInterface>();
      if(interface)
        {
        QVector3D vec = val.value<QVector3D>();
        interface->setOutputsPosition(prop, Eks::Vector3D(vec.x(), vec.y(), vec.z()));
        return true;
        }
      }
    else if(role == PropertyInputRole)
      {
      QModelIndex inputIndex = val.value<QModelIndex>();
      Property *input = (Property *)inputIndex.internalPointer();

      input->connect(prop);
      }
    }
  return false;
  }

bool DatabaseModel::setData(const QModelIndex &index, const QString &role, const QVariant &value)
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

QVariant DatabaseModel::headerData(int section, Qt::Orientation orientation, int role) const
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

QHash<int, QByteArray> DatabaseModel::roleNames() const
  {
  return _roles;
  }

Qt::ItemFlags DatabaseModel::flags(const QModelIndex &index) const
  {
  SDataModelProfileFunction
  Property *prop = (Property *)index.internalPointer();
  xAssert(!_currentTreeChange || _currentTreeChange->property() != prop);
  if(prop && index.column() < 2)
    {
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }
  return QAbstractItemModel::flags(index);
  }

void DatabaseModel::onTreeChange(const Change *c, bool back)
  {
  const Entity::TreeChange *tC = c->castTo<Entity::TreeChange>();
  if(tC)
    {
    xAssert(!_currentTreeChange);
    _currentTreeChange = tC;

    if(tC->property() == _root && tC->after(back) == 0)
      {
      _root = 0;
      }

    Q_EMIT layoutAboutToBeChanged();

    if(tC->after(back) == 0)
      {
      changePersistentIndex(createIndex(tC->index(), 0, tC->property()), QModelIndex());

      const PropertyContainer *parent = tC->before(back);
      xAssert(parent);

      xsize i = tC->index();
      Q_EMIT beginRemoveRows(createIndex(parent->parent()->index(parent), 0, (Property*)parent), i, i);
      _currentTreeChange = 0;
      Q_EMIT endRemoveRows();
      }
    else
      {
      const PropertyContainer *parent = tC->after(back);
      xAssert(parent);

      xsize i = xMin(parent->size()-1, tC->index());
      Q_EMIT beginInsertRows(createIndex(parent->parent()->index(parent), 0, (Property*)parent), i, i);
      _currentTreeChange = 0;
      Q_EMIT endInsertRows();
      }

    Q_EMIT layoutChanged();
    }

  const Property::NameChange *nameChange = c->castTo<Property::NameChange>();
  if(nameChange)
    {
    const Property *prop = nameChange->property();
    QModelIndex ind = createIndex(prop->parent()->index(prop), 0, (Property*)prop);
    Q_EMIT dataChanged(ind, ind);
    }
  }

void DatabaseModel::actOnChanges()
  {
  }

void DatabaseModel::setOptions(Options options)
  {
  Q_EMIT layoutAboutToBeChanged();
  _options = options;
  Q_EMIT layoutChanged();
  }

DatabaseModel::Options DatabaseModel::options() const
  {
  return _options;
  }

void DatabaseModel::setRoot(Entity *ent)
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

  Q_EMIT dataChanged(index(0, 0), index(_root->children.size(), 0));
  }

void DatabaseModel::setDatabase(Database *db, Entity *root)
  {
  _db = db;

  if(root == 0)
    {
    root = db;
    }

  setRoot(root);
  }

}
