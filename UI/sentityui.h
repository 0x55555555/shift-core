#ifndef SENTITYUI_H
#define SENTITYUI_H

#include "sglobal.h"
#include "XProperty"
#include "XMetaType"
#include "XList"
#include "QHash"

class QWidget;
class SEntity;
class SProperty;
class SPropertyInformation;

class SHIFT_EXPORT SEntityUI
  {
public:
  typedef QWidget *(*WidgetCreator)(const SProperty *property, bool readOnly, QWidget *parent);

XProperties:
  typedef XList<WidgetCreator> TypeList;
  typedef QHash<const SPropertyInformation *, int> LookupMap;
  XRORefProperty(TypeList, types);
  XRORefProperty(LookupMap, uiTypes);

public:
  enum InitiationOptions
    {
    NoDefaultUIHandlers = 1
    };

  SEntityUI(xuint32 options = 0);
  QWidget *createControlWidget(SEntity *ent, QWidget *parent=0, bool *anyWidgets=0) const;
  QWidget *createControlWidget(SProperty *property, QWidget *parent=0) const;
  bool canCreateWidget(SProperty *prop) const;

  template <typename T>int addWidgetCreator()
    {
    typedef T *(*TCreator)(SProperty *, bool, QWidget *);
    TCreator t = widgetCreatorHelper<T>;
    return addWidgetCreator((WidgetCreator)t);
    }

  int addWidgetCreator(WidgetCreator fn);
  void setUIType(const SPropertyInformation *type, int widgetType);

private:
  template <typename T> static T *widgetCreatorHelper(SProperty *prop, bool readOnly, QWidget *parent)
    {
    return new T(prop, readOnly, parent);
    }
  xuint64 widgetType(const SProperty *property) const;
  };

#endif // SENTITYUI_H
