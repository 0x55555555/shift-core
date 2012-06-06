#ifndef SLOADER_H
#define SLOADER_H

#include "sglobal.h"
#include "QHash"
#include "QTextStream"
#include "QDataStream"

class QString;
class SProperty;
class SPropertyContainer;
class SPropertyInformation;

class SLoader
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
  virtual ~SLoader() { }

  void setStreamDevice(Mode m, QIODevice *d) { _streamMode = m; _ts.setDevice(d); _ds.setDevice(d); }

  virtual const SPropertyInformation *type() const = 0;

  void loadChildren(SPropertyContainer *parent);

  virtual bool beginChildren() const = 0;
  virtual void endChildren() const = 0;
  virtual bool hasNextChild() const = 0;

  virtual void beginNextChild() = 0;
  virtual bool childHasValue() const = 0;
  virtual void endNextChild() = 0;

  void read(SPropertyContainer *parent);

  virtual void beginAttribute(const char *) = 0;
  virtual void endAttribute(const char *) = 0;

  virtual void resolveInputAfterLoad(SProperty *, const QString &) = 0;

  QTextStream &textStream() { return _ts; }
  QDataStream &binaryStream() { return _ds; }

private:
  QTextStream _ts;
  QDataStream _ds;
  };

class SSaver
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
  virtual ~SSaver() { }

  void setStreamDevice(Mode m, QIODevice *d) { _streamMode = m; _ts.setDevice(d); _ds.setDevice(d); }

  virtual void setType(const SPropertyInformation *) = 0;

  void saveChildren(const SPropertyContainer *c);

  virtual void beginChildren() = 0;
  virtual void endChildren() = 0;
  virtual void beginNextChild() = 0;
  virtual void endNextChild() = 0;

  void write(const SProperty *);

  virtual void beginAttribute(const char *) = 0;
  virtual void endAttribute(const char *) = 0;

  QTextStream &textStream() { return _ts; }
  QDataStream &binaryStream() { return _ds; }

private:
  QTextStream _ts;
  QDataStream _ds;
  };

template <typename T> void writeValue(SSaver &s, const T &t)
  {
  if(s.streamMode() == SSaver::Text)
    {
    s.textStream() << t;
    }
  else
    {
    s.binaryStream() << t;
    }
  }

template <typename T> void readValue(SLoader &l, T &t)
  {
  if(l.streamMode() == SLoader::Text)
    {
    l.textStream() >> t;
    }
  else
    {
    l.binaryStream() >> t;
    }
  }

inline void writeValue(SSaver &s, const QByteArray &t)
  {
  if(s.streamMode() == SSaver::Text)
    {
    s.textStream() << t.toHex();
    }
  else
    {
    s.binaryStream() << t;
    }
  }

inline void readValue(SLoader &l, QByteArray &t)
  {
  if(l.streamMode() == SLoader::Text)
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

inline void writeValue(SSaver &s, const QString &t)
  {
  if(s.streamMode() == SSaver::Text)
    {
    s.textStream() << t;
    }
  else
    {
    s.binaryStream() << t;
    }
  }

inline void readValue(SLoader &l, QString &t)
  {
  if(l.streamMode() == SLoader::Text)
    {
    t = l.textStream().readAll();
    }
  else
    {
    l.binaryStream() >> t;
    }
  }


#endif // SLOADER_H
