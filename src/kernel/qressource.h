#ifndef __QRESSOURCE_H__
#define __QRESSOURCE_H__

#include <qxmlparser.h>
#include <qstring.h>
#include <qwidget.h>
#include <qpixmap.h>
#include <qmap.h>
#include <qcolor.h>
#include <qstring.h>
#include <qfont.h>

class QRessource;

bool qXMLReadColor( const QString& _name, const QRessource& _res, QColor* color );
bool qXMLReadFont( const QString& _name, const QRessource& _res, QFont* font );
bool qXMLReadString( const QString& _name, const QRessource& _res, QString* str );
bool qXMLReadInt( const QString& _name, const QRessource& _res, int* i );
bool qXMLReadBool( const QString& _name, const QRessource& _res, bool* b );
bool qXMLReadDouble( const QString& _name, const QRessource& _res, double* d );
//bool qXMLReadAlign( const QString& _name, const QRessource& _res, int* align );

bool qXMLReadConnection( const QRessource& res, QObject* widget );

#define Q_RESSOURCE_ATTRIB( name, type, res, scan, inst, set ) { type x; if ( scan( name, res, &x ) ) inst->set( x ); }

class QRessourceFactory
{
public:
  typedef QWidget* (*WidgetCreator)( const QRessource& res, QWidget* _parent );
  typedef bool (*WidgetConfigurator)( const QRessource& res, QWidget* _instance );

  typedef QLayout* (*LayoutCreator)( const QRessource& res, QWidget* _parent );
  typedef bool (*LayoutConfigurator)( const QRessource& res, QLayout* _instance);

  typedef QObject* (*ObjectCreator)( const QRessource& res, QObject* _parent );
  typedef bool (*ObjectConfigurator)( const QRessource& res, QObject* _instance);

  virtual ~QRessourceFactory();

  virtual QObject* incarnateObject( const QRessource& res, QObject* _parent ) const;
  virtual QWidget* incarnateWidget( const QRessource& res, QWidget* _parent ) const;
  virtual QLayout* incarnateLayout( const QRessource& res, QWidget* _parent ) const;

  virtual QObject* createObject( const QString& _class, QObject* _parent ) const;
  virtual QWidget* createWidget( const QString& _class, QWidget* _parent ) const;
  virtual QLayout* createLayout( const QString& _class, QWidget* _parent ) const;

  virtual bool supportsType( const QString& _type ) const;

  void addFactory( const QString& _tag, WidgetCreator, WidgetConfigurator );
  void addFactory( const QString& _tag, LayoutCreator, LayoutConfigurator );
  void addFactory( const QString& _tag, ObjectCreator, ObjectConfigurator );

  static QRessourceFactory* factory();

protected:
  QRessourceFactory();

  struct WidgetFactory
  {
    WidgetFactory() { }
    WidgetFactory( const WidgetFactory& f ) { creator = f.creator; configurator = f.configurator; }
    WidgetFactory( WidgetCreator _cr, WidgetConfigurator _cf ) { creator = _cr; configurator = _cf; }
    WidgetCreator creator;
    WidgetConfigurator configurator;
  };

  struct LayoutFactory
  {
    LayoutFactory() { }
    LayoutFactory( const LayoutFactory& f ) { creator = f.creator; configurator = f.configurator; }
    LayoutFactory( LayoutCreator _cr, LayoutConfigurator _cf ) { creator = _cr; configurator = _cf; }
    LayoutCreator creator;
    LayoutConfigurator configurator;
  };
 
  struct ObjectFactory
  {
    ObjectFactory() { }
    ObjectFactory( const ObjectFactory& f ) { creator = f.creator; configurator = f.configurator; }
    ObjectFactory( ObjectCreator _cr, ObjectConfigurator _cf ) { creator = _cr; configurator = _cf; }
    ObjectCreator creator;
    ObjectConfigurator configurator;
  };
 
  QMap<QString,WidgetFactory> widgetFactories;
  QMap<QString,LayoutFactory> layoutFactories;
  QMap<QString,ObjectFactory> objectFactories;

  static QRessourceFactory* self;
};

class QRessource
{
public:
  QRessource();
  QRessource( const QRessource& );
  QRessource( const QString& _filename );
  virtual ~QRessource();

  bool setContent( const QString& _text );

  QObject* incarnateObject( QObject* _parent ) const;
  QWidget* incarnateWidget( QWidget* _parent ) const;
  QLayout* incarnateLayout( QWidget* _parent ) const;
  QPixmap incarnatePixmap() const;
  
  QRessource ressource( const QString& _name ) const;

  QRessource firstChild() const;
  QRessource nextSibling() const;
  QRessource parent() const;
  QRessource child( const QString& _type ) const;

  uint childCount() const { return it.childCount(); }

  bool hasParent() const;
  bool hasChildren() const;
  bool hasSibling() const;
  
  QString name() const;
  QString type() const;
  QString className() const;

  bool isText() const { return it.isText(); }
  QString text() const { return it.text(); }
  QString textAttrib( const QString& _name ) const { return it.textAttrib( _name ); }
  int intAttrib( const QString& _name ) const { return it.intAttrib( _name ); }
  double doubleAttrib( const QString& _name ) const { return it.doubleAttrib( _name ); }
  bool boolAttrib( const QString& _name ) const { return it.boolAttrib( _name ); }
  QColor colorAttrib( const QString& _name ) const { return it.colorAttrib( _name ); }
  bool hasAttrib( const QString& _name ) const { return it.hasAttrib( _name ); }

  bool isValid() const;

protected:
  QXMLIterator findRessource( const QString& _name ) const;

  QXMLIterator root() const;
  QXMLParseTree* parseTree();

private:
  QRessource( const QRessource&, QXMLIterator j );

  QXMLIterator findRessource( QXMLIterator j, const QString& _name ) const;

  QXMLIterator it;
  QXMLParseTree* sh;
};

#endif
