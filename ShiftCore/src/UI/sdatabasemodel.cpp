#include "shift/UI/sdatabasemodel.h"
#include "shift/Properties/scontaineriterators.h"
#include "shift/TypeInformation/sinterfaces.h"
#include "shift/sentity.h"
#include "shift/sdatabase.h"
#include "shift/Changes/spropertychanges.h"
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

CommonModel::CommonModel(Database *db, Entity *ent)
  : _db(db),
    _root(ent),
    _showValues(false)
  {
  if(_root == 0)
    {
    _root = db;
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

int CommonModel::columnCount( const QModelIndex &parent ) const
  {
  SDataModelProfileFunction
  const Property *prop = _root;
  if(parent.isValid())
    {
    prop = (Property *)parent.internalPointer();
    }

  if(_showValues && prop)
    {
    int columns = 1;

    const Container *cont = prop->castTo<Container>();
    if(cont)
      {
      xForeach(auto child, cont->walker())
        {
        // this could maybe be improved, but we dont want to show the values for complex widgets...
        const PropertyVariantInterface *ifc = child->findInterface<PropertyVariantInterface>();
        if(ifc)
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

QVariant CommonModel::data( const QModelIndex &index, int role ) const
  {
  SDataModelProfileFunction
  const Property *prop = (const Property *)index.internalPointer();
  if(!index.isValid())
    {
    if(role == PropertyColourRole)
      {
      const PropertyColourInterface *ifc = _root->findInterface<PropertyColourInterface>();
      if(ifc)
        {
        return ifc->colour(_root).toLDRColour();
        }
      return QColor();
      }
    return QVariant();
    }
  xAssert(prop);

  //xAssert(!_currentTreeChange || _currentTreeChange->property() != prop);

  if(role == Qt::DisplayRole)
    {
    if(_showValues && index.column() == 1)
      {
      const PropertyVariantInterface *ifc = prop->findInterface<PropertyVariantInterface>();
      if(ifc)
        {
        return ifc->asVariant(prop);
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
    const PropertyPositionInterface *ifc = prop->findInterface<PropertyPositionInterface>();
    if(ifc)
      {
      return toQt(ifc->position(prop));
      }
    return QVector3D();
    }
  else if(role == EntityInputPositionRole)
    {
    const PropertyPositionInterface *ifc = prop->findInterface<PropertyPositionInterface>();
    if(ifc)
      {
      return toQt(ifc->inputsPosition(prop));
      }
    return QVector3D();
    }
  else if(role == EntityOutputPositionRole)
    {
    const PropertyPositionInterface *ifc = prop->findInterface<PropertyPositionInterface>();
    if(ifc)
      {
      return toQt(ifc->outputsPosition(prop));
      }
    return QVector3D();
    }
  else if(role == PropertyColourRole)
    {
    const PropertyColourInterface *ifc = prop->findInterface<PropertyColourInterface>();
    if(ifc)
      {
      return ifc->colour(prop).toLDRColour();
      }
    return QColor();
    }
  else if(role == PropertyInputRole)
    {
    Property *inp = prop->input();
    if(inp)
      {
      return QVariant::fromValue(createIndex((int)inp->parent()->index(inp), 0, inp));
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

QVariant CommonModel::data( const QModelIndex &index, const QString &role) const
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

bool CommonModel::setData(const QModelIndex &index, const QVariant &val, int role)
  {
  //xAssert(!_currentTreeChange);
  SDataModelProfileFunction
  Property *prop = (Property *)index.internalPointer();
  if(prop)
    {
    if(role == Qt::DisplayRole)
      {
      if(_showValues && index.column() == 1)
        {
        PropertyVariantInterface *ifc = prop->findInterface<PropertyVariantInterface>();
        if(ifc)
          {
          ifc->setVariant(prop, val);
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
      PropertyPositionInterface *ifc = prop->findInterface<PropertyPositionInterface>();
      if(ifc)
        {
        QVector3D vec = val.value<QVector3D>();
        ifc->setPosition(prop, Eks::Vector3D(vec.x(), vec.y(), vec.z()));
        return true;
        }
      }
    else if(role == EntityInputPositionRole)
      {
      PropertyPositionInterface *ifc = prop->findInterface<PropertyPositionInterface>();
      if(ifc)
        {
        QVector3D vec = val.value<QVector3D>();
        ifc->setInputsPosition(prop, Eks::Vector3D(vec.x(), vec.y(), vec.z()));
        return true;
        }
      }
    else if(role == EntityOutputPositionRole)
      {
      PropertyPositionInterface *ifc = prop->findInterface<PropertyPositionInterface>();
      if(ifc)
        {
        QVector3D vec = val.value<QVector3D>();
        ifc->setOutputsPosition(prop, Eks::Vector3D(vec.x(), vec.y(), vec.z()));
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

bool CommonModel::setData(const QModelIndex &index, const QString &role, const QVariant &value)
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

QVariant CommonModel::headerData(int section, Qt::Orientation orientation, int role) const
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

QHash<int, QByteArray> CommonModel::roleNames() const
  {
  return _roles;
  }

QModelIndex CommonModel::index(Attribute *p) const
  {
  xAssert(p);
  Container *c = p->parent();
  if(!c)
    {
    return QModelIndex();
    }

  if(p == _root.entity())
    {
    return createIndex(0, 0, (Attribute *)p);
    }

  return createIndex((int)c->index(p), 0, (Attribute *)p);
  }

Attribute *CommonModel::attributeFromIndex(const QModelIndex &index) const
  {
  Attribute *prop = (Property *)index.internalPointer();
  return prop;
  }

Qt::ItemFlags CommonModel::flags(const QModelIndex &index) const
  {
  SDataModelProfileFunction
  Attribute *prop = attributeFromIndex(index);
  //xAssert(!_currentTreeChange || _currentTreeChange->property() != prop);
  if(prop && index.column() < 2)
    {
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }
  return QAbstractItemModel::flags(index);
  }

bool CommonModel::isEqual(const QModelIndex &a, const QModelIndex &b) const
  {
  const void *ap = attributeFromIndex(a);
  const void *bp = attributeFromIndex(b);
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

QModelIndex CommonModel::root() const
  {
  return createIndex(0, 0, (Property*)_root.entity());
  }

bool CommonModel::isValid(const QModelIndex &a) const
  {
  return a.isValid();
  }

int CommonModel::rowIndex(const QModelIndex &i) const
  {
  return i.row();
  }

int CommonModel::columnIndex(const QModelIndex &i) const
  {
  return i.column();
  }

void CommonModel::setRoot(Entity *ent)
  {
  beginResetModel();
  _root = ent;
  endResetModel();

  Q_EMIT dataChanged(index(0, 0), index((int)_root->children.size(), 0));
  }

void CommonModel::setDatabase(Database *db, Entity *root)
  {
  _db = db;

  if(root == 0)
    {
    root = db;
    }

  setRoot(root);
  }

DatabaseModel::DatabaseModel(Database *db, Entity *ent, Options options)
  : CommonModel(db, ent),
    _currentTreeChange(0),
    _options(options)
  {
  _showValues = (_options & ShowValues) != 0;

  if(_root.isValid())
    {
    _root->addTreeObserver(this);
    }
  }

DatabaseModel::~DatabaseModel()
  {
  if(_root.isValid())
    {
    _root->removeTreeObserver(this);
    }
  }

int DatabaseModel::rowCount( const QModelIndex &parent ) const
  {
  SDataModelProfileFunction
  const Attribute *prop = _root;
  if(parent.isValid())
    {
    prop = attributeFromIndex(parent);
    }

  if(!prop)
    {
    return 0;
    }

  if(_options.hasFlag(EntitiesOnly))
    {
    const Entity *ent = prop->uncheckedCastTo<Entity>();
    prop = &ent->children;
    }

  const Container *container = prop->castTo<Container>();
  if(container)
    {
    if(_currentTreeChange)
      {
      xAssert(container != _currentTreeChange->property());
      if(container == _currentTreeChange->after())
        {
        return (int)(container->size() - 1);
        }
      else if(container == _currentTreeChange->before())
        {
        return (int)(container->size() + 1);
        }
      }
    return (int)container->size();
    }

  return 0;
  }

QModelIndex DatabaseModel::index( int row, int column, const QModelIndex &parent ) const
  {
  SDataModelProfileFunction
  const Attribute *prop = _root;
  int size = 0;
  if(parent.isValid())
    {
    prop = attributeFromIndex(parent);
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

  const Container *container = prop->castTo<Container>();
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
        int newRow = xMin((int)(container->size()-1), (int)_currentTreeChange->index());
        if(row >= newRow)
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
    Attribute *prop = attributeFromIndex(child);
    Container *parent = prop->parent();

    if(_currentTreeChange)
      {
      if(prop == _currentTreeChange->property())
        {
        parent = (Container*)_currentTreeChange->before();
        }
      }

    if(parent)
      {
      if(_options.hasFlag(EntitiesOnly))
        {
        Entity *ent = parent->entity();
        return CommonModel::index(ent);
        }
      else
        {
        return CommonModel::index(parent);
        }
      }
    }
  return QModelIndex();
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
      changePersistentIndex(createIndex((int)tC->index(), 0, tC->property()), QModelIndex());

      const Container *parent = tC->before(back);
      xAssert(parent);

      xsize i = tC->index();
      Q_EMIT beginRemoveRows(createIndex((int)parent->parent()->index(parent), 0, (Property*)parent), (int)i, (int)i);
      _currentTreeChange = 0;
      Q_EMIT endRemoveRows();
      }
    else
      {
      const Container *parent = tC->after(back);
      xAssert(parent);

      int i = xMin((int)(parent->size()-1), (int)tC->index());
      Q_EMIT beginInsertRows(createIndex((int)parent->parent()->index(parent), 0, (Property*)parent), (int)i, (int)i);
      _currentTreeChange = 0;
      Q_EMIT endInsertRows();
      }

    Q_EMIT layoutChanged();
    }

  const Property::NameChange *nameChange = c->castTo<Property::NameChange>();
  if(nameChange)
    {
    const Attribute *prop = nameChange->attribute();
    QModelIndex ind = createIndex((int)prop->parent()->index(prop), 0, (Property*)prop);
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
  _showValues = (_options & ShowValues) != 0;
  Q_EMIT layoutChanged();
  }

DatabaseModel::Options DatabaseModel::options() const
  {
  return _options;
  }

void DatabaseModel::setRoot(Entity *ent)
  {
  if(_root)
    {
    _root->removeTreeObserver(this);
    }

  CommonModel::setRoot(ent);

  if(_root)
    {
    _root->addTreeObserver(this);
    }
  }

InputModel::InputModel(Database *db, Entity *ent, const PropertyInformation *itemType, const PropertyInformation *treeType, const EmbeddedPropertyInstanceInformation *childGroup)
  : CommonModel(db, ent),
    _itemType(itemType),
    _treeType(treeType),
    _childAttr(childGroup),
    _change(0)
  {
  if(_root.isValid())
    {
    _root->addConnectionObserver(this);
    }
  }

InputModel::~InputModel()
  {
  if(_root.isValid())
    {
    _root->removeConnectionObserver(this);
    }
  }


void InputModel::setRoot(Entity *ent)
  {
  if(_root)
    {
    _root->removeConnectionObserver(this);
    }

  CommonModel::setRoot(ent);

  if(_root)
    {
    _root->addConnectionObserver(this);
    }
  }

int InputModel::rowCount(const QModelIndex &parent) const
  {
  Attribute *attr = const_cast<Entity*>(_root->entity());
  if(parent.isValid())
    {
    attr = attributeFromIndex(parent);
    }


  xAssert(attr);
  const PropertyInformation *info = attr->typeInformation();
  xAssert(info->inheritsFromType(_itemType));

  Container *cont = attr->uncheckedCastTo<Container>();
  if(!info->inheritsFromType(_treeType))
    {
    return 0;
    }

  Attribute *children = _childAttr->locate(cont);
  Container *childrenCont = children->uncheckedCastTo<Container>();
  return (int)childrenCont->size();
  }

QModelIndex InputModel::index(int row, int, const QModelIndex &parent) const
  {
  Attribute *parentAttr = const_cast<Entity*>(_root.entity());
  if(parent.isValid())
    {
    parentAttr = attributeFromIndex(parent);
    }
  const PropertyInformation *info = parentAttr->typeInformation();
  xAssert(info->inheritsFromType(_treeType));

  Attribute *children = _childAttr->locate(parentAttr->uncheckedCastTo<Container>());
  Container *cont = children->uncheckedCastTo<Container>();
  if(row >= cont->size())
    {
    return QModelIndex();
    }

  Attribute *attr = cont->at(row);
  xAssert(attr);

  Property *prop = attr->uncheckedCastTo<Property>();
  if(!prop->input())
    {
    return CommonModel::index(prop);
    }

  return CommonModel::index(prop->input());
  }

QModelIndex InputModel::parent(const QModelIndex &child) const
  {
  Attribute *attr = attributeFromIndex(child);

  if(attr == _root.entity())
    {
    return QModelIndex();
    }

  if(!attr->typeInformation()->inheritsFromType(_itemType))
    {
    return CommonModel::index(attr->parent());
    }

  Property *prop = attr->uncheckedCastTo<Property>()->output();
  for(; prop; prop = prop->nextOutput())
    {
    Container *cont = prop->parent();
    if(!cont)
      {
      continue;
      }

    Container *tree = cont->parent();
    if(!tree)
      {
      continue;
      }

    if(tree->typeInformation()->inheritsFromType(_treeType))
      {
      return CommonModel::index(tree);
      }
    }

  xAssertFail();
  return QModelIndex();
  }

void InputModel::onConnectionChange(const Change *c, bool back)
  {
  const Property::ConnectionChange *_change = c->castTo<Property::ConnectionChange>();
  (void)_change;
  (void)back;
  }

void InputModel::actOnChanges()
  {
  }


}
