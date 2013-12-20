#include "shift/UI/sentityui.h"
#include "shift/UI/spropertydefaultui.h"
#include "shift/Properties/scontaineriterators.h"
#include "shift/Properties/sproperty.h"
#include "shift/Properties/scontainer.inl"
#include "shift/Properties/sdata.inl"
#include "shift/sentity.h"
#include "QtWidgets/QFormLayout"
#include "QtWidgets/QWidget"
#include "QtCore/QString"

namespace Shift
{

EntityUI::EntityUI(xuint32 options)
  {
  if((options&NoDefaultUIHandlers) != true)
    {
    setUIType(BoolProperty::staticTypeInformation(), addWidgetCreator<PropertyDefaultUI::Bool>() );

    setUIType(IntProperty::staticTypeInformation(), addWidgetCreator<PropertyDefaultUI::Int32>() );
    setUIType(UnsignedIntProperty::staticTypeInformation(), addWidgetCreator<PropertyDefaultUI::UInt32>() );
    setUIType(LongIntProperty::staticTypeInformation(), addWidgetCreator<PropertyDefaultUI::Int64>() );
    setUIType(LongUnsignedIntProperty::staticTypeInformation(), addWidgetCreator<PropertyDefaultUI::UInt64>() );

    setUIType(FloatProperty::staticTypeInformation(), addWidgetCreator<PropertyDefaultUI::Float>() );
    setUIType(DoubleProperty::staticTypeInformation(), addWidgetCreator<PropertyDefaultUI::Double>() );

    setUIType(StringProperty::staticTypeInformation(), addWidgetCreator<PropertyDefaultUI::String>() );

    setUIType(Vector2DProperty::staticTypeInformation(), addWidgetCreator<PropertyDefaultUI::Vector2D>() );
    setUIType(Vector3DProperty::staticTypeInformation(), addWidgetCreator<PropertyDefaultUI::Vector3D>() );
    setUIType(ColourProperty::staticTypeInformation(), addWidgetCreator<PropertyDefaultUI::Colour>() );
    }
  }

xuint64 EntityUI::widgetType(const Attribute *p) const
  {
  const PropertyInformation *actualType(p->typeInformation());
  if(_uiTypes.contains(actualType))
    {
    return _uiTypes[actualType];
    }
  return X_UINT64_SENTINEL;
  }

QWidget *EntityUI::createControlWidget(Entity *ent, QWidget *parent, bool *added) const
  {
  QWidget *canvas(new QWidget(parent));
  QFormLayout *layout(new QFormLayout(canvas));

  xForeach(auto child, ent->walker())
    {
    // this was too restricting, is it really what we want?
    //if(child->instanceInformation()->mode() == PropertyInstanceInformation::UserSettable)
      {
      QWidget *widget = createControlWidget(child);
      if(widget)
        {
        layout->addRow(child->identifier().toQString(), widget);
        if(added)
          {
          *added = true;
          }
        }
      }
    }
  return canvas;
  }

QWidget *EntityUI::createControlWidget(Attribute *property, QWidget *parent) const
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

bool EntityUI::canCreateWidget(Property *prop) const
  {
  xuint64 type = widgetType(prop);
  return type < (xuint64)_types.size();
  }

int EntityUI::addWidgetCreator(WidgetCreator fn)
  {
  _types << fn;
  return (int)(_types.size()-1);
  }

void EntityUI::setUIType(const PropertyInformation *type, int widgetType)
  {
  if(widgetType >= 0 && (xsize)widgetType < _types.size())
    {
    _uiTypes[type] = widgetType;
    }
  }

}
