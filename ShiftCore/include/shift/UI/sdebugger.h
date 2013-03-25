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

class SHIFT_EXPORT Debugger : public QWidget
  {
  Q_OBJECT

public:
  Debugger(Database *db, QWidget *parent=0);

private slots:
  void snapshot();

private:
  QGraphicsItem *createItemForProperty(Property *prop);

  QGraphicsScene *_scene;
  Database *_db;
  };

class DebugPropertyItem : public QGraphicsItem
  {
public:
  DebugPropertyItem(const QString &text);

  QRectF boundingRect() const;

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
  QStaticText _info;
  };

}

#endif // SDEBUGGER_H
