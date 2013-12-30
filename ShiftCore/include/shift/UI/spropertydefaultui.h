#ifndef SPROPERTYDEFAULTUI_H
#define SPROPERTYDEFAULTUI_H

#include "shift/sglobal.h"
#include "shift/Utilities/sentityweakpointer.h"
#include "shift/Properties/sproperty.h"
#include "shift/Properties/sbaseproperties.h"
#include "shift/Properties/sbaseproperties.inl"
#include "QtWidgets/QCheckBox"
#include "QtWidgets/QToolButton"
#include "QtWidgets/QSpinBox"
#include "QtWidgets/QLineEdit"
#include "QtWidgets/QTextEdit"
#include "QtWidgets/QFileDialog"
#include "QtWidgets/QHBoxLayout"
#include "XFloatWidget.h"
#include "XVector2DWidget.h"
#include "XVector3DWidget.h"
#include "XColourWidget.h"
#include "Utilities/XProperty.h"

namespace Shift
{

namespace PropertyDefaultUI
{
template <typename WIDG, typename T> class SUIBase : public WIDG, private DirtyObserver
  {
XProperties:
  XProperty(bool, isAlreadySetting, setAlreadySetting);

public:
  SUIBase(QWidget *parent, Property *p) : WIDG(parent), _isAlreadySetting(false), _value(p->castTo<T>()), _dirty(false)
    {
    xAssert(_value);
    _entity = _value->entity();
    xAssert(_entity);
    }
  ~SUIBase()
    {
    }

  T *propertyValue() {return _value;}
  virtual void syncGUI() = 0;

private:
  virtual void onPropertyDirtied(const Property* prop)
    {
    if(prop == _value)
      {
      _dirty = true;
      }
    }
  virtual void actOnChanges()
    {
    SProfileFunction
    if(_dirty && !_isAlreadySetting && WIDG::isVisible())
      {
      _isAlreadySetting = true;
      syncGUI();
      _isAlreadySetting = false;
      _dirty = true;
      }
    }

  void showEvent(QShowEvent *) X_OVERRIDE
    {
    actOnChanges();
    }

  T *_value;
  EntityWeakPointer _entity;
  bool _dirty;
  };

class SHIFT_EXPORT Bool : public SUIBase<QCheckBox, BoolProperty>
  {
  Q_OBJECT
public:
  Bool( Property *prop, bool readOnly, QWidget *parent) : SUIBase<QCheckBox, BoolProperty>(parent, prop)
    {
    connect( this, SIGNAL(toggled(bool)), this, SLOT(guiChanged(bool)));
    syncGUI();
    setDisabled(readOnly);
    }

  Q_SLOT void guiChanged( bool val ) { propertyValue()->assign(val); }
  void syncGUI() { setChecked( propertyValue()->value() ); }
  };

class SHIFT_EXPORT Int32 : public SUIBase<QSpinBox, IntProperty>
 {
 Q_OBJECT
public:
 Int32( Property *prop, bool readOnly, QWidget *parent) : SUIBase<QSpinBox, IntProperty>(parent, prop)
   {
   connect( this, SIGNAL(valueChanged(int)), this, SLOT(guiChanged(int)));
   setAlreadySetting(true);
   syncGUI();
   setAlreadySetting(false);
   setMinimum( -0x7FFFFFF );
   setMaximum( 0x7FFFFFF );
   setReadOnly(readOnly);
   }

 Q_SLOT void guiChanged( int val ) { propertyValue()->assign((xint32)val); }
 void syncGUI() { setValue(propertyValue()->value()); }
 };

class SHIFT_EXPORT UInt32 : public SUIBase<QSpinBox, UnsignedIntProperty>
 {
 Q_OBJECT
public:
 UInt32( Property *prop, bool readOnly, QWidget *parent) : SUIBase<QSpinBox, UnsignedIntProperty>(parent, prop)
   {
   connect( this, SIGNAL(valueChanged(int)), this, SLOT(guiChanged(int)));
   setAlreadySetting(true);
   syncGUI();
   setAlreadySetting(false);
   setMinimum( -0x7FFFFFF );
   setMaximum( 0x7FFFFFF );
   setReadOnly(readOnly);
   }

 Q_SLOT void guiChanged( int val ) { propertyValue()->assign((xuint32)val); }
 void syncGUI() { setValue(propertyValue()->value()); }
 };

class SHIFT_EXPORT Int64 : public SUIBase<QSpinBox, LongIntProperty>
 {
 Q_OBJECT
public:
 Int64( Property *prop, bool readOnly, QWidget *parent ) : SUIBase<QSpinBox, LongIntProperty>(parent, prop)
   {
   connect( this, SIGNAL(valueChanged(int)), this, SLOT(guiChanged(int)));
   setAlreadySetting(true);
   syncGUI();
   setAlreadySetting(false);
   setMinimum( -0x7FFFFFF );
   setMaximum( 0x7FFFFFF );
   setReadOnly(readOnly);
   }

 Q_SLOT void guiChanged( int val ) { propertyValue()->assign((xint64)val); }
 void syncGUI() { setValue(propertyValue()->value()); }
 };

class SHIFT_EXPORT UInt64 : public SUIBase<QSpinBox, LongUnsignedIntProperty>
 {
 Q_OBJECT
public:
 UInt64(Property *prop, bool readOnly, QWidget *parent) : SUIBase<QSpinBox, LongUnsignedIntProperty>(parent, prop)
   {
   connect( this, SIGNAL(valueChanged(int)), this, SLOT(guiChanged(int)));
   setAlreadySetting(true);
   syncGUI();
   setAlreadySetting(false);
   setMinimum( -0x7FFFFFF );
   setMaximum( 0x7FFFFFF );
   setReadOnly(readOnly);
   }

 Q_SLOT void guiChanged( int val ) { propertyValue()->assign((xuint64)val); }
 void syncGUI() { setValue(propertyValue()->value()); }
 };

class SHIFT_EXPORT Float : public SUIBase<XFloatWidget, FloatProperty>
  {
  Q_OBJECT
public:
  Float(Property *prop, bool readOnly, QWidget *parent) : SUIBase<XFloatWidget, FloatProperty>(parent, prop)
    {
    connect( this, SIGNAL(valueChanged(double)), this, SLOT(guiChanged(double)));
    setReadOnly(readOnly);

    blockSignals(true);
    syncGUI();
    blockSignals(false);
    }

  Q_SLOT void guiChanged( double val ) { propertyValue()->assign((float)val); }
  void syncGUI() { setValue(propertyValue()->value()); }
  };

class SHIFT_EXPORT Double : public SUIBase<XFloatWidget, DoubleProperty>
  {
  Q_OBJECT
public:
  Double(Property *prop, bool readOnly, QWidget *parent) : SUIBase<XFloatWidget, DoubleProperty>(parent, prop)
    {
    connect( this, SIGNAL(valueChanged(double)), this, SLOT(guiChanged(double)));
    setReadOnly(readOnly);

    blockSignals(true);
    syncGUI();
    blockSignals(false);
    }

  Q_SLOT void guiChanged( double val ) { propertyValue()->assign(val); }
  void syncGUI() { setValue( propertyValue()->value() ); }
  };

class SHIFT_EXPORT String : public SUIBase<QLineEdit, StringProperty>
  {
  Q_OBJECT
public:
  String(Property *prop, bool readOnly, QWidget *parent) : SUIBase<QLineEdit, StringProperty>(parent, prop)
    {
    connect( this, SIGNAL(editingFinished()), this, SLOT(guiChanged()) );
    syncGUI();
    setReadOnly(readOnly);
    }

  Q_SLOT virtual void guiChanged( ) { propertyValue()->assign(text()); }
  void syncGUI() { setText(propertyValue()->value().toQString()); }
  };

class SHIFT_EXPORT LongString : public SUIBase<QTextEdit, StringProperty>
  {
  Q_OBJECT
public:
  LongString(Property *prop, bool readOnly, QWidget *parent) : SUIBase<QTextEdit, StringProperty>(parent, prop)
    {
    connect( this, SIGNAL(textChanged()), this, SLOT(guiChanged()) );
    syncGUI();
    setAcceptRichText( false );
    setReadOnly(readOnly);
    }

  Q_SLOT virtual void guiChanged( ) { propertyValue()->assign(toPlainText());}
  void syncGUI()
    {
    QString newValue = propertyValue()->value().toQString();
    if(toPlainText() != newValue)
      {
      setText(newValue);
      }
    }
  };

class SHIFT_EXPORT Vector2D : public SUIBase<XVector2DWidget, Vector2DProperty>
  {
  Q_OBJECT
public:
  Vector2D(Property *prop, bool readOnly, QWidget *parent) : SUIBase<XVector2DWidget, Vector2DProperty>(parent, prop)
    {
    connect( this, SIGNAL(valueChanged(Eks::Vector2D)), this, SLOT(guiChanged(Eks::Vector2D)));
    setReadOnly(readOnly);
    }

private Q_SLOTS:
  void guiChanged( Eks::Vector2D val ) { propertyValue()->assign(val); }

private:
  void syncGUI() { setValue( propertyValue()->value() ); }
  };


class SHIFT_EXPORT Vector3D : public SUIBase<XVector3DWidget, Vector3DProperty>
  {
  Q_OBJECT
public:
  Vector3D(Property *prop, bool readOnly, QWidget *parent) : SUIBase<XVector3DWidget, Vector3DProperty>(parent, prop)
    {
    connect( this, SIGNAL(valueChanged(XVector3D)), this, SLOT(guiChanged(XVector3D)));
    setReadOnly(readOnly);
    syncGUI();
    }

private Q_SLOTS:
  void guiChanged( Eks::Vector3D val ) { propertyValue()->assign(val); }

private:
  void syncGUI() { setValue( propertyValue()->value() ); }
  };

class SHIFT_EXPORT Colour : public SUIBase<XColourWidget, ColourProperty>
  {
  Q_OBJECT
public:
  Colour(Property *prop, bool X_UNUSED(readOnly), QWidget *parent) : SUIBase<XColourWidget, ColourProperty>(parent, prop)
    {
    connect( this, SIGNAL(colourChanged(XColour)), this, SLOT(guiChanged(XColour)));
    // setReadOnly(readOnly); <- implement this...
    syncGUI();
    }
private Q_SLOTS:
  virtual void guiChanged( const Eks::Colour &col ) { propertyValue()->assign(col); }

private:
  void syncGUI() { setColour( propertyValue()->value() ); }
  };

#if 0
class Filename : public SUIBase<QWidget, FilenameProperty>
  {
  Q_OBJECT
public:
  Filename(Property *prop, bool X_UNUSED(readOnly), QWidget *parent) : SUIBase<QWidget, FilenameProperty>(parent, prop),
      _layout( new QHBoxLayout( this ) ), _label( new QLineEdit( this ) ),
      _button( new QToolButton( this ) )
    {
    _layout->setContentsMargins( 0, 0, 0, 0 );
    _layout->addWidget( _label );
    _layout->addWidget( _button );

    _label->setReadOnly( true );
    _label->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    _button->setText( "..." );

    connect( _button, SIGNAL(clicked()), this, SLOT(guiChanged()) );
    syncGUI();
    }
private Q_SLOTS:
  virtual void guiChanged( )
    {
    //QSettings settings;
    QString file( QFileDialog::getOpenFileName( 0, "Select File for " + propertyValue()->identifier().toQString() ) );

    propertyValue()->assign(file);

    //QFileInfo fileInfo( file );
    //settings.setValue( "lastDirAccessed", fileInfo.absoluteDir().absolutePath() );
    }
  virtual void syncGUI()
    {
    _label->setText(propertyValue()->value().toQString());
    }
private:
  QHBoxLayout *_layout;
  QLineEdit *_label;
  QToolButton *_button;
  };

  /** \brief APrivateVector2DProperty Sets and displays a GUI for a 3D vector.
    */
  class APrivateVector2DProperty : public QWidget
      {
      Q_OBJECT
  public:
      /** create a vectorProperty */
      APrivateVector2DProperty( AProperty *d )
              : _stack( new QStackedWidget( this ) ),
              _vec( new Eks::Vector2DWidget( d->value().toVector2D() ) ),
              _sca( new XFloatWidget( d->value().toVector2D().x() ) )
          {
          setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Fixed );

          QHBoxLayout *layout( new QHBoxLayout( this ) );
          layout->setContentsMargins( 0, 0, 0, 0 );

          _button = new QToolButton( this );
          _button->setIconSize( QSize( 10, 10 ) );
          _button->setCheckable( true );
          _button->setAutoRaise( true );

          QVBoxLayout *buttonLayout( new QVBoxLayout( ) );
          buttonLayout->addWidget( _button );
          buttonLayout->addStretch();

          layout->addLayout( buttonLayout );
          layout->addWidget( _stack );

          _stack->addWidget( _sca );
          _stack->addWidget( _vec );
          data = d;

          toggleMode( data->attributes()["expandGUIMode"].toInt() );

          connect( _button, SIGNAL(clicked(bool)), this, SLOT(toggleMode(bool)) );
          connect( data, SIGNAL(onPropertyChange(AProperty*)), this, SLOT(propertyChanged(AProperty*)) );
          connect( _vec, SIGNAL(valueChanged(Eks::Vector2D)), this, SLOT(guiChanged(Eks::Vector2D)));
          connect( _sca, SIGNAL(valueChanged(double)), this, SLOT(guiChanged(double)));
          }
      virtual QSize sizeHint() const
          {
          return QSize( QWidget::sizeHint().width(), maximumHeight() );
          }
  private Q_SLOTS:
      void toggleMode( bool b )
          {
          _button->setChecked( b );
          _stack->setCurrentIndex( b );
          setMaximumHeight( _stack->currentWidget()->sizeHint().height() );
          data->attribute( "expandGUIMode" ) = (int)b;

          if( b )
              {
              _button->setIcon( QIcon( ":/app/close.svg") );
              }
          else
              {
              _button->setIcon( QIcon( ":/app/open.svg") );
              }
          }
      void guiChanged( Eks::Vector2D val )
          { *data = val; }
      void guiChanged( double val )
          { *data = Eks::Vector2D( val, val ); }
      void propertyChanged( AProperty *in )
          {
          _vec->setValue( in->value().toVector2D() );
          _sca->setValue( in->value().toDouble() );
          }
  private:
      QToolButton *_button;
      AProperty *data;
      QStackedWidget *_stack;
      Eks::Vector2DWidget *_vec;
      XFloatWidget *_sca;
      };

  /** \brief APrivateVector4DProperty Sets and displays a GUI for a 3D vector.
    */
  class APrivateVector4DProperty : public QWidget
      {
      Q_OBJECT
  public:
      /** create a vectorProperty */
      APrivateVector4DProperty( AProperty *d )
              : _stack( new QStackedWidget( this ) ),
              _vec( new XVector4DWidget( d->value().toVector4D() ) ),
              _sca( new XFloatWidget( d->value().toVector4D().x() ) )
          {
          setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Fixed );

          QHBoxLayout *layout( new QHBoxLayout( this ) );
          layout->setContentsMargins( 0, 0, 0, 0 );

          _button = new QToolButton( this );
          _button->setIconSize( QSize( 10, 10 ) );
          _button->setCheckable( true );
          _button->setAutoRaise( true );

          QVBoxLayout *buttonLayout( new QVBoxLayout( ) );
          buttonLayout->addWidget( _button );
          buttonLayout->addStretch();

          layout->addLayout( buttonLayout );
          layout->addWidget( _stack );

          _stack->addWidget( _sca );
          _stack->addWidget( _vec );
          data = d;

          toggleMode( data->attributes()["expandGUIMode"].toInt() );

          connect( _button, SIGNAL(clicked(bool)), this, SLOT(toggleMode(bool)) );
          connect( data, SIGNAL(onPropertyChange(AProperty*)), this, SLOT(propertyChanged(AProperty*)) );
          connect( _vec, SIGNAL(valueChanged(Eks::Vector4D)), this, SLOT(guiChanged(Eks::Vector4D)));
          connect( _sca, SIGNAL(valueChanged(double)), this, SLOT(guiChanged(double)));
          }
      virtual QSize sizeHint() const
          {
          return QSize( QWidget::sizeHint().width(), maximumHeight() );
          }
  private Q_SLOTS:
      void toggleMode( bool b )
          {
          _button->setChecked( b );
          _stack->setCurrentIndex( b );
          setMaximumHeight( _stack->currentWidget()->sizeHint().height() );
          data->attribute( "expandGUIMode" ) = (int)b;

          if( b )
              {
              _button->setIcon( QIcon( ":/app/close.svg") );
              }
          else
              {
              _button->setIcon( QIcon( ":/app/open.svg") );
              }
          }
      void guiChanged( Eks::Vector4D val )
          { *data = val; }
      void guiChanged( double val )
          { *data = Eks::Vector4D( val, val, val, val ); }
      void propertyChanged( AProperty *in )
          {
          _vec->setValue( in->value().toVector4D() );
          _sca->setValue( in->value().toDouble() );
          }
  private:
      QToolButton *_button;
      AProperty *data;
      QStackedWidget *_stack;
      XVector4DWidget *_vec;
      XFloatWidget *_sca;
      };

  /** \brief APrivateColourProperty Sets and displays a GUI colour value.
    */
  class APrivateColourProperty : public XColourWidget
      {
      Q_OBJECT
  public:
      /** Create a colourProperty
        */
      APrivateColourProperty( AProperty *d, bool alpha=TRUE ) : XColourWidget( d->value().toColour(), alpha )
          {
          data = d;
          connect( data, SIGNAL(onPropertyChange(AProperty*)), this, SLOT(propertyChanged(AProperty*)) );
          connect( this, SIGNAL(colourChanged(XColour)), this, SLOT(guiChanged(XColour)));
          }
  private Q_SLOTS:
      virtual void guiChanged( XColour col )
          { *data = col; }
      virtual void propertyChanged( AProperty *in )
          { setColour( in->value().toColour() ); }
  private:
      AProperty *data;
      };

  /** \brief APrivateFileProperty Sets and displays a GUI file value.
    */
  class APrivateFileProperty : public QWidget
      {
      Q_OBJECT
  public:
      /** Create a fileProperty
        */
      APrivateFileProperty( AProperty *d ) : _layout( new QHBoxLayout( this ) ), _label( new QLineEdit( this ) ),
              _button( new QToolButton( this ) )
          {
          data = d;

          _layout->setContentsMargins( 0, 0, 0, 0 );
          _layout->addWidget( _label );
          _layout->addWidget( _button );

          _label->setReadOnly( TRUE );
          _label->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
          _button->setText( "..." );

          connect( data, SIGNAL(onPropertyChange(AProperty*)), this, SLOT(propertyChanged(AProperty*)) );
          connect( _button, SIGNAL(clicked()), this, SLOT(guiChanged()) );
          propertyChanged( d );
          }
  private Q_SLOTS:
      virtual void guiChanged( )
          {
          QSettings settings;
          QString file( AFileDialog::getOpenFileName( "Select File for " + data->displayName() ) );
          *data = file;
          QFileInfo fileInfo( file );
          settings.setValue( "lastDirAccessed", fileInfo.absoluteDir().absolutePath() );
          }
      virtual void propertyChanged( AProperty *in )
          {
          _label->setText( in->value().toString() );
          }
  private:
      AProperty *data;
      QHBoxLayout *_layout;
      QLineEdit *_label;
      QToolButton *_button;
      };

  /** \brief APrivateDirectoryProperty Sets and displays a GUI file value.
    */
  class APrivateDirectoryProperty : public QWidget
      {
      Q_OBJECT
  public:
      /** Create a directoryProperty
        */
      APrivateDirectoryProperty( AProperty *d ) : _layout( new QHBoxLayout( this ) ), _label( new QLineEdit( this ) ),
              _button( new QToolButton( this ) )
          {
          data = d;

          _layout->setContentsMargins( 0, 0, 0, 0 );
          _layout->addWidget( _label );
          _layout->addWidget( _button );

          _label->setReadOnly( TRUE );
          _label->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
          _button->setText( "..." );

          connect( data, SIGNAL(onPropertyChange(AProperty*)), this, SLOT(propertyChanged(AProperty*)) );
          connect( _button, SIGNAL(clicked()), this, SLOT(guiChanged()) );
          propertyChanged( d );
          }
  private Q_SLOTS:
      virtual void guiChanged( )
          {
          QSettings settings;
          QString file( AFileDialog::getExistingDirectory( "Select Directory for " + data->displayName() ) );
          *data = file;
          settings.setValue( "lastDirAccessed", file );
          }
      virtual void propertyChanged( AProperty *in )
          {
          _label->setText( in->value().toString() );
          }
  private:
      AProperty *data;
      QHBoxLayout *_layout;
      QLineEdit *_label;
      QToolButton *_button;
      };

  /** \brief APrivateFileSequenceProperty creates a gui property for file sequences
    */
  class APrivateFileSequenceProperty : public QWidget
      {
      Q_OBJECT
  public:
      /** Create a file sequence property
        */
      APrivateFileSequenceProperty( AProperty *d ) : _layout( new QHBoxLayout( this ) ),
              _label( new QLineEdit( this ) ), _button( new QToolButton( this ) )
          {
          data = d;

          _layout->setContentsMargins( 0, 0, 0, 0 );
          _layout->addWidget( _label );
          _layout->addWidget( _button );

          _label->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
          _button->setText( "..." );

          connect( data, SIGNAL(onPropertyChange(AProperty*)), this, SLOT(propertyChanged(AProperty*)) );
          connect( _button, SIGNAL(clicked()), this, SLOT(guiChanged()) );
          connect( _label, SIGNAL(editingFinished()), this, SLOT(guiForceChanged()) );
          propertyChanged( d );
          }

  private Q_SLOTS:
      void guiForceChanged()
          {
          *data = XFileSequence( _label->text(), XFileSequence::Parsed );
          }

      void guiChanged( )
          {
          QString fN( AFileDialog::getOpenFileName( "Choose a file from the sequence" ) );
          data->attribute("chosenFile") = fN;
          ALog << "File Sequence" << fN << data->attributes()["chosenFile"];
          *data = XFileSequence( fN, XFileSequence::Auto );
          }

      void propertyChanged( AProperty *in )
          {
          _label->setText( XFileSequence( in->value() ).parsedFilename() );
          }

  private:
      AProperty *data;
      QHBoxLayout *_layout;
      QLineEdit *_label;
      QToolButton *_button;
      };
  }
#endif
}

}

#endif // SPROPERTYDEFAULTUI_H
