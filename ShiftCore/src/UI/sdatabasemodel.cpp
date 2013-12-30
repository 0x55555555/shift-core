#include "shift/UI/sdatabasemodel.h"
#include "shift/Properties/scontaineriterators.h"
#include "shift/Properties/scontainer.inl"
#include "shift/Properties/sattribute.inl"
#include "shift/Properties/sbasepointerproperties.h"
#include "shift/TypeInformation/sinterfaces.h"
#include "shift/sentity.h"
#include "shift/sdatabase.h"
#include "shift/Changes/spropertychanges.h"
#include "QtWidgets/QPushButton"
#include "QtWidgets/QStyleOptionViewItem"

Q_DECLARE_METATYPE(QModelIndex)

namespace Shift
{

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
      QString name = prop->identifier().toQString();
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

  xsize idx = c->index(p);

  return index(p, idx);
  }

QModelIndex CommonModel::index(Attribute *p, xsize row) const
  {
  xAssert(p);
  if(p == _root.entity())
    {
    return createIndex(0, 0, (Attribute *)p);
    }

  return createIndex((int)row, 0, (Attribute *)p);
  }

Attribute *CommonModel::attributeFromIndex(const QModelIndex &index) const
  {
  Attribute *prop = (Property *)index.internalPointer();
  return prop;
  }

Qt::ItemFlags CommonModel::flags(const QModelIndex &index) const
  {
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
    _options(options),
    _currentTreeChange(0)
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
  const ContainerTreeChange *tC = c->castTo<ContainerTreeChange>();
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

      int i = (int)xMin(parent->size()-1, tC->index());
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
    _holderChanging(0),
    _itemChanging(0),
    _itemIndexChanging(X_SIZE_SENTINEL),
    _changingAdding(false)
  {
  if(_root.isValid())
    {
    manageObserver(_root.entity(), true);
    }
  }

InputModel::~InputModel()
  {
  if(_root.isValid())
    {
    manageObserver(_root.entity(), false);
    }
  }


void InputModel::setRoot(Entity *ent)
  {
  if(_root)
    {
    manageObserver(_root.entity(), false);
    }

  CommonModel::setRoot(ent);

  if(_root)
    {
    manageObserver(_root.entity(), true);
    }
  }

void InputModel::manageObserver(Attribute *attr, bool add)
  {
  const PropertyInformation *info = attr->typeInformation();
  if(!info->inheritsFromType(_treeType))
    {
    return;
    }

  Entity *ent = attr->entity();
  xAssert(ent);
  if(add)
    {
    ent->addConnectionObserver(this);
    }
  else
    {
    ent->removeConnectionObserver(this);
    }

  Container *c = attr->castTo<Container>();
  if(!c)
    {
    xAssertFail();
    return;
    }

  Attribute *children = _childAttr->locate(c);
  if(!children)
    {
    xAssertFail();
    return;
    }

  Container *childCont = children->castTo<Container>();
  if(!childCont)
    {
    xAssertFail();
    return;
    }

  xForeach(auto child, childCont->walker<Shift::Pointer>())
    {
    if(child->pointed())
      {
      manageObserver(child->pointed(), add);
      }
    }
  }

int InputModel::rowCount(const QModelIndex &parent) const
  {
  if(!_root)
    {
    return 0;
    }
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

  int extra = 0;
  if(_holderChanging == attr)
    {
    if(_changingAdding)
      {
      extra = -1;
      }
    else
      {
      extra = 1;
      }
    }

  Attribute *children = _childAttr->locate(cont);
  Container *childrenCont = children->uncheckedCastTo<Container>();
  return (int)childrenCont->size() + extra;
  }

QModelIndex InputModel::index(int row, int, const QModelIndex &parent) const
  {
  Attribute *parentAttr = const_cast<Entity*>(_root.entity());
  if(parent.isValid())
    {
    parentAttr = attributeFromIndex(parent);
    }
  X_USED_FOR_ASSERTS(const PropertyInformation *info =)
    parentAttr->typeInformation();
  xAssert(info->inheritsFromType(_treeType));

  Attribute *children = _childAttr->locate(parentAttr->uncheckedCastTo<Container>());
  Container *cont = children->uncheckedCastTo<Container>();
  if((xsize)row >= cont->size())
    {
    return QModelIndex();
    }

  if(_holderChanging == parentAttr)
    {
    if(_changingAdding)
      {
      if((xsize)row >= _itemIndexChanging)
        {
        row += 1;
        }
      }
    else
      {
      if((xsize)row == _itemIndexChanging)
        {
        return CommonModel::index((Container *)_itemChanging);
        }
      else if((xsize)row > _itemIndexChanging)
        {
        row -= 1;
        }
      }
    }

  Attribute *attr = cont->at(row);
  xAssert(attr);

  Property *prop = attr->uncheckedCastTo<Property>();
  if(!prop->input())
    {
    return CommonModel::index(prop);
    }

  return CommonModel::index(prop->input(), row);
  }

QModelIndex InputModel::parent(const QModelIndex &child) const
  {
  Attribute *attr = attributeFromIndex(child);

  if(attr == _root.entity())
    {
    return QModelIndex();
    }

  if(attr == _itemChanging)
    {
    return CommonModel::index((Container *)_holderChanging);
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

    if(tree == _root.entity())
      {
      return QModelIndex();
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
  if(!_root)
    {
    return;
    }

  const Property::ConnectionChange *change = c->castTo<Property::ConnectionChange>();
  if(change)
    {
    const Property *src = change->driver();
    const Property *dest = change->driven();

    if(!src->typeInformation()->inheritsFromType(_itemType))
      {
      return;
      }

    const Container *dstContainer = dest->parent();
    xAssert(dstContainer);
    const Container *dstContatinerHolder = dstContainer->parent();
    xAssert(dstContatinerHolder);
    if(!dstContatinerHolder->typeInformation()->inheritsFromType(_treeType))
      {
      return;
      }

    if(_childAttr->locate(dstContatinerHolder) != dstContainer)
      {
      return;
      }

    xsize i = dstContainer->index(dest);

    _holderChanging = dstContatinerHolder;
    _itemChanging = src;
    _itemIndexChanging = i;
    _changingAdding = change->mode(back) == Property::ConnectionChange::Connect;
    Q_EMIT layoutAboutToBeChanged();

    if(change->mode(back) == Property::ConnectionChange::Disconnect)
      {
      changePersistentIndex(CommonModel::index((Property*)src), QModelIndex());
      Q_EMIT beginRemoveRows(CommonModel::index((Property*)dstContatinerHolder), (int)i, (int)i);

      _holderChanging = 0;
      _itemChanging = 0;
      _itemIndexChanging = 0;

      manageObserver(const_cast<Property*>(src), false);

      Q_EMIT endRemoveRows();
      }
    else if(change->mode(back) == Property::ConnectionChange::Connect)
      {
      Q_EMIT beginInsertRows(CommonModel::index((Property*)dstContatinerHolder), (int)i, (int)i);

      _holderChanging = 0;
      _itemChanging = 0;
      _itemIndexChanging = 0;

      manageObserver(const_cast<Property*>(src), true);

      Q_EMIT endInsertRows();
      }

    Q_EMIT layoutChanged();
    }
  }

void InputModel::onTreeChange(const Change *c, bool)
  {
  const Property::NameChange *nameChange = c->castTo<Property::NameChange>();
  if(nameChange)
    {
    const Attribute *prop = nameChange->attribute();
    QModelIndex ind = CommonModel::index((Attribute*)prop);
    Q_EMIT dataChanged(ind, ind);
    }
  }

void InputModel::actOnChanges()
  {
  }


}
