#ifndef SDEBUGGER_H
#define SDEBUGGER_H

#include "shift/sglobal.h"
#include "QtWidgets/QWidget"
#include "QtWidgets/QGraphicsItem"
#include "QtGui/QStaticText"

class QGraphicsScene;

#ifdef X_DEBUG
# define S_DEBUGGER
#endif

namespace Shift
{

class Database;
class DebugPropertyItem;

class SHIFT_EXPORT Debugger : public QWidget
  {
  Q_OBJECT

public:
  Debugger(Database *db, QWidget *parent=0);

private slots:
  void snapshot();


private:
  DebugPropertyItem *createItemForProperty(Attribute *prop, Eks::UnorderedMap<Attribute *, DebugPropertyItem *> *itemsOut = 0);
  void connectProperties(const Eks::UnorderedMap<Attribute *, DebugPropertyItem *> &itemsOut);

  QGraphicsScene *_scene;
  Database *_db;
  };

class DebugPropertyItem : public QGraphicsObject
  {
  Q_OBJECT
public:
  DebugPropertyItem(const QString &text, const QColor &colour);

  QRectF boundingRect() const;
  QRectF boundingRectWithChildProperties() const;

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

  enum { Type = UserType + 1 };
  int type() const { return Type; }

signals:
  void showConnected();

public slots:
  void layout();
  void show();

protected slots:
  void hide();
  void hideChildren();
  void hideSiblings();
  void showChildren();
  void isolate();

protected:
  void mouseMoveEvent(QGraphicsSceneMouseEvent * event) X_OVERRIDE;
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) X_OVERRIDE;

private:
  QStaticText _info;
  QColor _outerColour;
  };

class ConnectionItem : public QGraphicsObject
  {
  Q_OBJECT
public:
  ConnectionItem(DebugPropertyItem *from, DebugPropertyItem *owner, bool horizontal, QColor colour);

  QRectF boundingRect() const;

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

  enum { Type = UserType + 2 };
  int type() const { return Type; }

private slots:
  void updateEndPoints();

private:
  void points(QPointF &from, QPointF &to) const;
  DebugPropertyItem *_owner;
  DebugPropertyItem *_from;

  bool _horizontal;

  QColor _colour;
  };

}

#endif // SDEBUGGER_H
