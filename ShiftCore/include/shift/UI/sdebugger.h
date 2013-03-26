#ifndef SDEBUGGER_H
#define SDEBUGGER_H

#include "shift/sglobal.h"
#include "QtWidgets/QWidget"
#include "QtWidgets/QGraphicsItem"
#include "QtGui/QStaticText"

class QGraphicsScene;

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
  DebugPropertyItem *createItemForProperty(Property *prop, Eks::UnorderedMap<Property *, DebugPropertyItem *> *itemsOut = 0);
  void connectProperties(const Eks::UnorderedMap<Property *, DebugPropertyItem *> &itemsOut);

  QGraphicsScene *_scene;
  Database *_db;
  };

class DebugPropertyItem : public QGraphicsObject
  {
  Q_OBJECT
public:
  DebugPropertyItem(const QString &text, const QColor &colour);

  QRectF boundingRect() const;
  
  float layout();

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected slots:
  void hideChildren();
  void showChildren();
  
protected:
  void mouseMoveEvent(QGraphicsSceneMouseEvent * event) X_OVERRIDE;

private:
  QStaticText _info;
  QColour _outerColour;
  };

class ConnectionItem : public QGraphicsObject
  {
  Q_OBJECT
public:
  ConnectionItem(DebugPropertyItem *from, DebugPropertyItem *owner, QColor colour);

  QRectF boundingRect() const;

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private slots:
  void updateEndPoints();

private:
  DebugPropertyItem *_owner;
  DebugPropertyItem *_from;

  QColor _colour;
  };

}

#endif // SDEBUGGER_H
