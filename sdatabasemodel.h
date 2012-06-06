#ifndef SDATABASEMODEL_H
#define SDATABASEMODEL_H

#include "sglobal.h"
#include "QAbstractItemModel"
#include "QItemDelegate"
#include "sobserver.h"
#include "UI/sentityui.h"
#include "sentityweakpointer.h"
#include "XFlags"

class SDatabase;

class SHIFT_EXPORT SDatabaseDelegate : public QItemDelegate
  {
  Q_OBJECT

public:
  SDatabaseDelegate(QObject *parent);

  virtual QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
  virtual void setEditorData( QWidget *editor, const QModelIndex &index ) const;
  virtual void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;
  QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

private slots:
  void currentItemDestroyed();

private:
  SEntityUI _ui;
  mutable QWidget *_currentWidget;
  mutable QModelIndex _currentIndex;
  };

class SHIFT_EXPORT SDatabaseModel : public QAbstractItemModel, STreeObserver
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

  SDatabaseModel(SDatabase *db, SEntity *ent, Options options);
  ~SDatabaseModel();

  QModelIndex index(const SProperty *) const;

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

  virtual void onTreeChange(const SChange *, bool back);
  virtual void actOnChanges();

  void setOptions(Options options);
  Options options() const;

  virtual Qt::ItemFlags flags(const QModelIndex &index) const;

  void setRoot(SEntity *ent);
  void setDatabase(SDatabase *db, SEntity *ent);

private:
  SDatabase *_db;
  SEntityWeakPointer _root;
  Options _options;
  const SPropertyContainer::TreeChange *_currentTreeChange;
  };

#endif // SDATABASEMODEL_H
