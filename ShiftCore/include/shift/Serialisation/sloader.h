#ifndef SLOADER_H
#define SLOADER_H

#include "shift/sglobal.h"
#include "QHash"
#include "QTextStream"
#include "QDataStream"

class QString;

namespace Shift
{

class Property;
class PropertyContainer;
class PropertyInformation;

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

  virtual const PropertyInformation *type() const = 0;

  void loadChildren(PropertyContainer *parent);

  virtual bool beginChildren() const = 0;
  virtual void endChildren() const = 0;
  virtual bool hasNextChild() const = 0;

  virtual void beginNextChild() = 0;
  virtual bool childHasValue() const = 0;
  virtual void endNextChild() = 0;

  void read(PropertyContainer *parent);

  virtual void beginAttribute(const char *) = 0;
  virtual void endAttribute(const char *) = 0;

  typedef Eks::StringBase<Eks::Char, 1024> InputString;
  virtual void resolveInputAfterLoad(Property *, const InputString &) = 0;

  QTextStream &textStream() { return _ts; }
  QDataStream &binaryStream() { return _ds; }

private:
  QTextStream _ts;
  QDataStream _ds;
  };

class Saver
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
  virtual ~Saver() { }

  void setStreamDevice(Mode m, QIODevice *d) { _streamMode = m; _ts.setDevice(d); _ds.setDevice(d); }

  virtual void setType(const PropertyInformation *) = 0;

  void saveChildren(const PropertyContainer *c);

  virtual void beginChildren() = 0;
  virtual void endChildren() = 0;
  virtual void beginNextChild() = 0;
  virtual void endNextChild() = 0;

  void write(const Property *);

  virtual void beginAttribute(const char *) = 0;
  virtual void endAttribute(const char *) = 0;

  QTextStream &textStream() { return _ts; }
  QDataStream &binaryStream() { return _ds; }

private:
  QTextStream _ts;
  QDataStream _ds;
  };

template <typename T> void writeValue(Saver &s, const T &t)
  {
  if(s.streamMode() == Saver::Text)
    {
    s.textStream() << t;
    }
  else
    {
    s.binaryStream() << t;
    }
  }

template <typename T> void readValue(Loader &l, T &t)
  {
  if(l.streamMode() == Loader::Text)
    {
    l.textStream() >> t;
    }
  else
    {
    l.binaryStream() >> t;
    }
  }

inline void writeValue(Saver &s, const QByteArray &t)
  {
  if(s.streamMode() == Saver::Text)
    {
    s.textStream() << t.toHex();
    }
  else
    {
    s.binaryStream() << t;
    }
  }

inline void readValue(Loader &l, QByteArray &t)
  {
  if(l.streamMode() == Loader::Text)
    {
    QByteArray temp;
    l.textStream() >> temp;
    t = QByteArray::fromHex(temp);
    }
  else
    {
    l.binaryStream() >> t;
    }
  }

template <xsize Size, typename Alloc>
inline void writeValue(Saver &s, const Eks::StringBase<Eks::Char, Size, Alloc> &t)
  {
  if(s.streamMode() == Saver::Text)
    {
    s.textStream() << t.toQString();
    }
  else
    {
    s.binaryStream() << t.toQString();
    }
  }

template <xsize Size, typename Alloc>
inline void readValue(Loader &l, Eks::StringBase<Eks::Char, Size, Alloc> &t)
  {
  if(l.streamMode() == Loader::Text)
    {
    QString arr = l.textStream().readAll();
    t = arr;
    }
  else
    {
    QString v;
    l.binaryStream() >> v;
    t = v;
    }
  }

}

#endif // SLOADER_H
