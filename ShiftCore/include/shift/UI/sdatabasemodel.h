#ifndef SDATABASEMODEL_H
#define SDATABASEMODEL_H

#include "shift/sglobal.h"
#include "shift/Utilities/sentityweakpointer.h"
#include "shift/UI/sentityui.h"
#include "shift/Changes/sobserver.h"
#include "QtCore/QAbstractItemModel"
#include "QtWidgets/QStyledItemDelegate"
#include "Utilities/XFlags.h"

namespace Shift
{

class Database;

class SHIFT_EXPORT DatabaseDelegate : public QStyledItemDelegate
  {
  Q_OBJECT

public:
  DatabaseDelegate(QObject *parent);

  virtual QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
  virtual void setEditorData( QWidget *editor, const QModelIndex &index ) const;
  virtual void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;
  QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

private Q_SLOTS:
  void currentItemDestroyed();

private:
  EntityUI _ui;
  mutable QWidget *_currentWidget;
  mutable QModelIndex _currentIndex;
  };

class SHIFT_EXPORT CommonModel : public QAbstractItemModel
  {
public:
  enum
    {
    PropertyPositionRole = Qt::UserRole,
    PropertyColourRole,
    PropertyInputRole,
    PropertyModeRole,
    IsEntityRole,
    EntityInputPositionRole,
    EntityOutputPositionRole
    };

  CommonModel(Database *db, Entity *ent);

  using QAbstractItemModel::index;
  QModelIndex index(Attribute *p) const;
  QModelIndex index(Attribute *p, xsize row) const;

  Attribute *attributeFromIndex(const QModelIndex &index) const;

  virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
  Q_INVOKABLE QVariant data( const QModelIndex &index, const QString &role) const;
  Q_INVOKABLE bool setData(const QModelIndex & index, const QString &role, const QVariant & value);
  virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;
  bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  Q_INVOKABLE QModelIndex root() const;
  Q_INVOKABLE bool isEqual(const QModelIndex &a, const QModelIndex &b) const;
  Q_INVOKABLE bool isValid(const QModelIndex &i) const;
  Q_INVOKABLE int rowIndex(const QModelIndex &i) const;
  Q_INVOKABLE int columnIndex(const QModelIndex &i) const;

  virtual Qt::ItemFlags flags(const QModelIndex &index) const;

  QHash<int, QByteArray> roleNames() const X_OVERRIDE;

  virtual void setRoot(Entity *ent);
  void setDatabase(Database *db, Entity *ent);

protected:
  Database *_db;
  EntityWeakPointer _root;
  QHash<int, QByteArray> _roles;
  bool _showValues;
  };

class SHIFT_EXPORT DatabaseModel : public CommonModel, TreeObserver
  {
  Q_OBJECT
public:
  enum OptionsFlags
    {
    NoOptions = 0,
    EntitiesOnly = 1,
    //DisableUpdates = 2,
    ShowValues = 4
    };
  typedef Eks::Flags<OptionsFlags> Options;

  DatabaseModel(Database *db, Entity *ent, Options options);
  ~DatabaseModel();

  Q_INVOKABLE virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const X_OVERRIDE;
  Q_INVOKABLE virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const X_OVERRIDE;
  Q_INVOKABLE virtual QModelIndex parent( const QModelIndex &child ) const X_OVERRIDE;

  void onTreeChange(const Change *, bool back) X_OVERRIDE;
  void actOnChanges() X_OVERRIDE;

  void setOptions(Options options);
  Options options() const;

  void setRoot(Entity *ent) X_OVERRIDE;

private:
  Options _options;
  const ContainerTreeChange *_currentTreeChange;
  };


class SHIFT_EXPORT InputModel : public CommonModel, ConnectionObserver, TreeObserver
  {
  Q_OBJECT
public:
  InputModel(Database *db, Entity *ent, const PropertyInformation *itemType, const PropertyInformation *treeType, const EmbeddedPropertyInstanceInformation *childGroup);
  ~InputModel();

  Q_INVOKABLE virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const X_OVERRIDE;
  Q_INVOKABLE virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const X_OVERRIDE;
  Q_INVOKABLE virtual QModelIndex parent( const QModelIndex &child ) const X_OVERRIDE;

  void onConnectionChange(const Change *, bool back) X_OVERRIDE;
  void onTreeChange(const Change *, bool back) X_OVERRIDE;
  void actOnChanges() X_OVERRIDE;

  void setRoot(Entity *ent) X_OVERRIDE;

private:
  const PropertyInformation *_itemType;
  const PropertyInformation *_treeType;
  const EmbeddedPropertyInstanceInformation *_childAttr;

  void manageObserver(Attribute *attr, bool add);

  const Container *_holderChanging;
  const Property *_itemChanging;
  xsize _itemIndexChanging;
  bool _changingAdding;
  };

}

#endif // SDATABASEMODEL_H
