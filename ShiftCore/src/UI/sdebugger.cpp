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
  Eks::TemporaryAllocator alloc(Shift::TypeRegistry::temporaryAllocator());
  Eks::UnorderedMap<Property *, DebugPropertyItem *> props(&alloc);
  QGraphicsItem *snapshot = createItemForProperty(_db, &props);
  
  connectProperties(props);

  _scene->addItem(snapshot);
  }
  
void connectProperties(const Eks::UnorderedMap<Property *, DebugPropertyItem *> &itemsOut)
  {
  Eks::UnorderedMap<Property *, DebugPropertyItem *>::iterator it = itemsOut.begin();
  Eks::UnorderedMap<Property *, DebugPropertyItem *>::iterator end = itemsOut.end();
  for(; it != end; ++it)
    {
    Property *p = it->first;
    if(p->input())
      {
      const auto &inItem = itemsOut[p->input()];
      xAssert(inItem);
      
      new ConnectionItem(inItem, p, Qt::red);
      }
    }
  }

DebugPropertyItem *Debugger::createItemForProperty(Property *prop, Eks::UnorderedMap<Property *, DebugPropertyItem *> *itemsOut)
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
  
  PropertyContainer *c = prop->castTo<PropertyContainer>();
  if(c)
    {
    int count = c->size();
    int embCount = c->typeInformation->childCount();
    text += "<br>embedded children: " + QString::number(embCount);
    text += "<br>dynamic children: " + QString::number(count-embCount);
    }

  QColor colour = Qt::black;
  if(prop->isDynamic())
    {
    colour = Qt::blue;
    }

  DebugPropertyItem *item = new DebugPropertyItem(text, colour);
  if(itemsOut)
    {
    (*itemsOut)[prop] = item;
    }

  PropertyContainer *c = prop->castTo<PropertyContainer>();
  if(c)
    {
    xForeach(auto p, c->walker())
      {
      DebugPropertyItem *childItem = createItemForProperty(p, itemsOut);
      childItem->setParentItem(item);

      new ConnectionItem(item, childItem, Qt::black);
      }
    }

  item->setPos(width()/2, height()/2);
  item->layout();
  
  return item;
  }

DebugPropertyItem::DebugPropertyItem(const QString &text, const QColor &colour)
    : _info(text), _colour(colour)
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

void DebugPropertyItem::contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
  {
  QMenu menu;
  menu.addAction("Hide Children", this, SLOT(hideChildren()));
  menu.addAction("Show Children", this, SLOT(showChildren())));
  
  menu.popup();
  }
  
void DebugPropertyItem::hideChildren()
  {
  xForeach(auto child, childItems())
    {
    child->setVisible(false);
    }
  }
  
void DebugPropertyItem::showChildren()
  {
  xForeach(auto child, childItems())
    {
    child->setVisible(true);
    }
  }

void DebugPropertyItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
  {
  painter->setPen(_colour);
  painter->setBrush(Qt::white);

  QSizeF size = _info.size();
  painter->drawRoundedRect(-5, -5, size.width()+10, size.height()+10, 2, 2);

  painter->setPen(Qt::black);
  painter->drawStaticText(0, 0, _info);
  }
  
float DebugPropertyItem::layout()
  {
  float childWidth = 0.0f;
  xsize children = 0;
  xForeach(auto child, childItems())
    {
    DebugPropertyItem *childItem = qgraphicsitem_cast<DebugPropertyItem*>(child);
    if(childItem)
      {
      childWidth += childItem->layout();
      ++children;
      }
    }
    
  enum
    {
    GapX = 5,
    ChildOffsetY = 50
    };
    
  float fullWidth = childWidth + ((chilren-1) * GapX);
  
  float currentX = fullWidth/2.0f;
  xForeach(auto child, childItems())
    {
    DebugPropertyItem *childItem = qgraphicsitem_cast<DebugPropertyItem*>(child);
    if(childItem)
      {
      childItem->setPos(currentX, ChildOffsetY);
      currentX += childItem->boundingRect().width() + GapX;
      }
    }
    
  return fullWidth;
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
