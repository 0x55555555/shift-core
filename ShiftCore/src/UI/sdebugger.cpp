#include "shift\UI\sdebugger.h"
#include "QtWidgets/QToolBar"
#include "QtWidgets/QVBoxLayout"
#include "QtWidgets/QGraphicsView"
#include "QtWidgets/QGraphicsScene"
#include "QtWidgets/QGraphicsSceneMouseEvent"
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
  view->setRenderHints(QPainter::Antialiasing);
  view->setDragMode(QGraphicsView::ScrollHandDrag);
  }

void Debugger::snapshot()
  {
  QGraphicsItem *snapshot = createItemForProperty(_db);

  _scene->addItem(snapshot);
  }

DebugPropertyItem *Debugger::createItemForProperty(Property *prop)
  {
  QString text = "name: " + prop->name().toQString() + "<br>type: " + prop->typeInformation()->typeName().toQString();
  PropertyVariantInterface *ifc = prop->interface<PropertyVariantInterface>();
  if(ifc)
    {
    NoUpdateBlock b(prop);
    text += "<br>value: " + ifc->asString(prop).toQString();
    }

  bool dirty = prop->isDirty();
  text += "<br>dirty: ";
  text += (dirty ? "true" : "false");

  enum
    {
    OffsetX = 20,
    GapX = 5,
    ChildOffsetY = 50
    };

  DebugPropertyItem *item = new DebugPropertyItem(text);

  PropertyContainer *c = prop->castTo<PropertyContainer>();
  if(c)
    {
    xsize posX = OffsetX;
    xForeach(auto p, c->walker())
      {
      DebugPropertyItem *childItem = createItemForProperty(p);
      childItem->setPos(posX, ChildOffsetY);
      childItem->setParentItem(item);
      posX += GapX + childItem->boundingRect().width();

      new ConnectionItem(item, childItem, Qt::black);
      }
    }

  item->setPos(width()/2, height()/2);
  return item;
  }

DebugPropertyItem::DebugPropertyItem(const QString &text)
    : _info(text)
  {
  setAcceptedMouseButtons(Qt::LeftButton);
  setFlag(QGraphicsItem::ItemIsMovable);
  }

QRectF DebugPropertyItem::boundingRect() const
  {
  qreal penWidth = 1;
  QSizeF size = _info.size();

  QRectF bnds(-5 - (penWidth / 2), - 5 - (penWidth / 2), size.width() + 10 + penWidth, size.height() + 10  + penWidth);

  return bnds;
  }

void DebugPropertyItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
  {
  QPointF initialPos = event->lastScenePos();
  QPointF nowPos = event->scenePos();

  QPointF diff = nowPos - initialPos;

  setPos(pos() + diff);
  }

void DebugPropertyItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
  {
  painter->setPen(QPen(Qt::black));
  painter->setBrush(Qt::white);

  QSizeF size = _info.size();
  painter->drawRoundedRect(-5, -5, size.width()+10, size.height()+10, 2, 2);

  painter->drawStaticText(0, 0, _info);
  }

ConnectionItem::ConnectionItem(DebugPropertyItem *from, DebugPropertyItem *owner, QColor col)
  : QGraphicsObject(owner), _colour(col), _from(from), _owner(owner)
  {
  xAssert(_owner);
  xAssert(_from);

  connect(_owner, SIGNAL(xChanged()), this, SLOT(updateEndPoints()));
  connect(_owner, SIGNAL(yChanged()), this, SLOT(updateEndPoints()));
  connect(_from, SIGNAL(xChanged()), this, SLOT(updateEndPoints()));
  connect(_from, SIGNAL(yChanged()), this, SLOT(updateEndPoints()));
  }

void ConnectionItem::updateEndPoints()
  {
  prepareGeometryChange();
  update();
  }

QRectF ConnectionItem::boundingRect() const
  {
  QPointF ptA = mapFromItem(_owner, 0, 0);
  QPointF ptB = mapFromItem(_from, 0, 0);

  QSizeF s(2, 2);

  return QRectF(ptA, s) | QRectF(ptB, s);
  }

void ConnectionItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
  {
  painter->setPen(QPen(Qt::black));

  QPointF ptA = mapFromItem(_owner, 0, 0);
  QPointF ptB = mapFromItem(_from, 0, 0);

  painter->drawLine(ptA, ptB);
  }

}
