#ifndef SLOADER_H
#define SLOADER_H

#include "shift/sglobal.h"
#include "shift/Utilities/spropertyname.h"
#include "Utilities/XProperty.h"
#include "Containers/XStringSimple.h"
#include "QtCore/QTextStream"
#include "QtCore/QDataStream"

class QString;

namespace Shift
{

class Property;
class Container;
class PropertyInformation;

class LoadInterface
  {
public:

  };

class LoadBuilder : public LoadInterface
  {
XProperties:
  XROProperty(Container *, root);

public:
  LoadBuilder(Container *root);
  };

/*
class Loader
  {
public:
  enum Mode
    {
    Text,
    Binary
    };

XProperties:
  XROProperty(Mode, streamMode);

public:
  virtual ~Loader() { }

  void setStreamDevice(Mode m, QIODevice *d) { _streamMode = m; _ts.setDevice(d); _ds.setDevice(d); }

  struct CurrentData
    {
    Attribute *existing;
    const PropertyInformation *type;
    Name name;
    bool dynamic;
    };

  virtual const CurrentData *currentData() const { return &_data; }

  void loadChildren(Container *parent);

  virtual bool beginChildren() const = 0;
  virtual void endChildren() const = 0;
  virtual bool hasNextChild() const = 0;

  virtual void beginNextChild() = 0;
  virtual bool childHasValue() const = 0;
  virtual void endNextChild() = 0;

  void read(Container *parent);

  virtual void beginAttribute(const char *) = 0;
  virtual void endAttribute(const char *) = 0;

  typedef Eks::StringBase<Eks::Char, 1024> InputString;
  virtual void resolveInputAfterLoad(Property *, const InputString &) = 0;

  QTextStream &textStream() { return _ts; }
  QDataStream &binaryStream() { return _ds; }

private:
  QTextStream _ts;
  QDataStream _ds;

  CurrentData _data;
  };*/


}

#endif // SLOADER_H
