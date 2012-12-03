#ifndef SDATABASEMODEL_H
#define SDATABASEMODEL_H

#include "shift/sglobal.h"
#include "shift/Utilities/sentityweakpointer.h"
#include "shift/UI/sentityui.h"
#include "shift/Changes/sobserver.h"
#include "QAbstractItemModel"
#include "QItemDelegate"
#include "XFlags"

namespace Shift
{

class Database;

class SHIFT_EXPORT DatabaseDelegate : public QItemDelegate
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

class SHIFT_EXPORT DatabaseModel : public QAbstractItemModel, TreeObserver
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
  typedef XFlags<OptionsFlags> Options;

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

  DatabaseModel(Database *db, Entity *ent, Options options);
  ~DatabaseModel();

  QModelIndex index(const Property *) const;

  Q_INVOKABLE QModelIndex root() const;
  Q_INVOKABLE bool isEqual(const QModelIndex &a, const QModelIndex &b) const;
  Q_INVOKABLE bool isValid(const QModelIndex &i) const;
  Q_INVOKABLE int rowIndex(const QModelIndex &i) const;
  Q_INVOKABLE int columnIndex(const QModelIndex &i) const;

  Q_INVOKABLE virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
  Q_INVOKABLE virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
  Q_INVOKABLE virtual QModelIndex parent( const QModelIndex &child ) const;
  virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;
  virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
  Q_INVOKABLE QVariant data( const QModelIndex &index, const QString &role) const;
  bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
  Q_INVOKABLE bool setData(const QModelIndex & index, const QString &role, const QVariant & value);
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  virtual void onTreeChange(const Change *, bool back);
  virtual void actOnChanges();

  void setOptions(Options options);
  Options options() const;

  virtual Qt::ItemFlags flags(const QModelIndex &index) const;

  void setRoot(Entity *ent);
  void setDatabase(Database *db, Entity *ent);

private:
  Database *_db;
  EntityWeakPointer _root;
  Options _options;
  const PropertyContainer::TreeChange *_currentTreeChange;
  };

}

#endif // SDATABASEMODEL_H
