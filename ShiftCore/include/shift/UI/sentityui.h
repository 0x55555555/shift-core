#ifndef SENTITYUI_H
#define SENTITYUI_H

#include "shift/sglobal.h"
#include "XProperty"
#include "XUnorderedMap"

class QWidget;

namespace Shift
{

class Entity;
class Property;
class PropertyInformation;

class SHIFT_EXPORT EntityUI
  {
public:
  typedef QWidget *(*WidgetCreator)(const Attribute *property, bool readOnly, QWidget *parent);

XProperties:
  typedef Eks::Vector<WidgetCreator> TypeList;
  typedef Eks::UnorderedMap<const PropertyInformation *, int> LookupMap;
  XRORefProperty(TypeList, types);
  XRORefProperty(LookupMap, uiTypes);

public:
  enum InitiationOptions
    {
    NoDefaultUIHandlers = 1
    };

  EntityUI(xuint32 options = 0);
  QWidget *createControlWidget(Entity *ent, QWidget *parent=0, bool *anyWidgets=0) const;
  QWidget *createControlWidget(Attribute *property, QWidget *parent=0) const;
  bool canCreateWidget(Property *prop) const;

  template <typename T>int addWidgetCreator()
    {
    typedef T *(*TCreator)(Property *, bool, QWidget *);
    TCreator t = widgetCreatorHelper<T>;
    return addWidgetCreator((WidgetCreator)t);
    }

  int addWidgetCreator(WidgetCreator fn);
  void setUIType(const PropertyInformation *type, int widgetType);

private:
  template <typename T> static T *widgetCreatorHelper(Property *prop, bool readOnly, QWidget *parent)
    {
    return new T(prop, readOnly, parent);
    }
  xuint64 widgetType(const Attribute *property) const;
  };

}

#endif // SENTITYUI_H
