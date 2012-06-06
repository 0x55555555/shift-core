#include "sentityui.h"
#include "QFormLayout"
#include "QWidget"
#include "spropertydefaultui.h"
#include "QString"
#include "sentity.h"
#include "sproperty.h"

SEntityUI::SEntityUI(xuint32 options)
  {
  if((options&NoDefaultUIHandlers) != true)
    {
    setUIType(BoolProperty::staticTypeInformation(), addWidgetCreator<SPropertyDefaultUI::Bool>() );

    setUIType(IntProperty::staticTypeInformation(), addWidgetCreator<SPropertyDefaultUI::Int32>() );
    setUIType(UnsignedIntProperty::staticTypeInformation(), addWidgetCreator<SPropertyDefaultUI::UInt32>() );
    setUIType(LongIntProperty::staticTypeInformation(), addWidgetCreator<SPropertyDefaultUI::Int64>() );
    setUIType(LongUnsignedIntProperty::staticTypeInformation(), addWidgetCreator<SPropertyDefaultUI::UInt64>() );

    setUIType(FloatProperty::staticTypeInformation(), addWidgetCreator<SPropertyDefaultUI::Float>() );
    setUIType(DoubleProperty::staticTypeInformation(), addWidgetCreator<SPropertyDefaultUI::Double>() );

    setUIType(StringProperty::staticTypeInformation(), addWidgetCreator<SPropertyDefaultUI::String>() );

    setUIType(Vector2DProperty::staticTypeInformation(), addWidgetCreator<SPropertyDefaultUI::Vector2D>() );
    setUIType(Vector3DProperty::staticTypeInformation(), addWidgetCreator<SPropertyDefaultUI::Vector3D>() );
    setUIType(ColourProperty::staticTypeInformation(), addWidgetCreator<SPropertyDefaultUI::Colour>() );

    setUIType(FilenameProperty::staticTypeInformation(), addWidgetCreator<SPropertyDefaultUI::Filename>() );
    }
  }

xuint64 SEntityUI::widgetType(const SProperty *p) const
  {
  const SPropertyInformation *actualType(p->typeInformation());
  if(_uiTypes.contains(actualType))
    {
    return _uiTypes[actualType];
    }
  return X_UINT64_SENTINEL;
  }

QWidget *SEntityUI::createControlWidget(SEntity *ent, QWidget *parent, bool *added) const
  {
  QWidget *canvas(new QWidget(parent));
  QFormLayout *layout(new QFormLayout(canvas));

  SProperty *child = ent->firstChild();
  while(child)
    {
    // this was too restricting, is it really what we want?
    //if(child->instanceInformation()->mode() == SPropertyInstanceInformation::UserSettable)
      {
      QWidget *widget = createControlWidget(child);
      if(widget)
        {
        layout->addRow(child->name(), widget);
        if(added)
          {
          *added = true;
          }
        }
      }
    child = child->nextSibling();
    }
  return canvas;
  }

QWidget *SEntityUI::createControlWidget(SProperty *property, QWidget *parent) const
  {
  xuint64 type = widgetType(property);
  if(type < (xuint64)_types.size())
    {
    bool readOnly = false;
    // todo, make read only customisable..
    return _types[type](property, readOnly, parent);
    }
  return 0;
  }

bool SEntityUI::canCreateWidget(SProperty *prop) const
  {
  xuint64 type = widgetType(prop);
  return type < (xuint64)_types.size();
  }

int SEntityUI::addWidgetCreator(WidgetCreator fn)
  {
  _types << fn;
  return _types.size()-1;
  }

void SEntityUI::setUIType(const SPropertyInformation *type, int widgetType)
  {
  if(widgetType >= 0 && widgetType < _types.size())
    {
    _uiTypes[type] = widgetType;
    }
  }
