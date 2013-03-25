#include "shift\UI\sdebugger.h"
#include "QtWidgets/QToolBar"
#include "QtWidgets/QVBoxLayout"
#include "QtWidgets/QGraphicsView"
#include "QtWidgets/QGraphicsScene"
#include "shift/sdatabase.h"
#include "shift/Properties/sproperty.h"
#include "shift/Properties/spropertycontaineriterators.h"
#include "shift/TypeInformation/sinterfaces.h"

namespace Shift
{

Debugger::Debugger(Shift::Database *db, QWidget *parent) : QWidget(parent)
  {
  _db = db;

  move(10, 10);

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);

  QToolBar *toolbar = new QToolBar("mainTool", this);
  layout->addWidget(toolbar);

  toolbar->addAction("Snapshot", this, SLOT(snapshot()));

  _scene = new QGraphicsScene(this);

  _scene->setBackgroundBrush(QColor(64, 64, 64));

  QGraphicsView *view = new QGraphicsView(_scene, this);
  layout->addWidget(view);
  }

void Debugger::snapshot()
  {
  QGraphicsItem *snapshot = createItemForProperty(_db);

  _scene->addItem(snapshot);
  }

QGraphicsItem *Debugger::createItemForProperty(Property *prop)
  {
  QString text = prop->name().toQString();
  PropertyVariantInterface *ifc = prop->interface<PropertyVariantInterface>();
  if(ifc)
    {
    text + "\nvalue: " + ifc->asString(prop).toQString();
    }

  enum
    {
    ChildOffsetX = 100,
    ChildOffsetY = 50
    };

  DebugPropertyItem *item = new DebugPropertyItem(text);

  PropertyContainer *c = prop->castTo<PropertyContainer>();
  if(c)
    {
    xsize index = 0;
    xForeach(auto p, c->walker())
      {
      QGraphicsItem *childItem = createItemForProperty(p);
      childItem->setPos(index * ChildOffsetX, ChildOffsetY);
      childItem->setParentItem(item);
      }
    }

  item->setPos(width()/2, height()/2);
  return item;
  }

DebugPropertyItem::DebugPropertyItem(const QString &text)
    : _info(text)
  {
  setAcceptedMouseButtons(Qt::LeftButton)
  }

QRectF DebugPropertyItem::boundingRect() const
  {
  qreal penWidth = 1;
  return QRectF(-10 - penWidth / 2, -10 - penWidth / 2,
                20 + penWidth, 20 + penWidth);
  }

void DebugPropertyItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
  {
  painter->setPen(QPen(Qt::red));
  painter->setBrush(Qt::blue);
  painter->drawRoundedRect(-10, -10, 20, 20, 5, 5);
  }

}
