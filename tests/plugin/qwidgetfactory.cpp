#include "qwidgetfactory.h"
#include <qdict.h>
#include <qfile.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdial.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qhbuttongroup.h>
#include <qheader.h>
#include <qhgroupbox.h>
#include <qiconview.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qmultilineedit.h>
#include <qpopupmenu.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qscrollbar.h>
#include <qscrollview.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qstatusbar.h>
#include <qtabbar.h>
#include <qtabwidget.h>
#include <qtextbrowser.h>
#include <qtextview.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qvbox.h>
#include <qvbuttongroup.h>
#include <qvgroupbox.h>
#include <qwidgetstack.h>
#include <qworkspace.h>

static const unsigned int prime[6] = { 53, 151, 503, 1511, 5101, 15101 };
static int primeSize = 0;
static QDict<QWidgetFactory> factories( prime[0] );

/*!
  \class QWidgetFactory qwidgetfactory.h
  \brief Factory-class for widgets.
*/

/*!
  Installs a QWidgetFactory.
  Gets the list of widgets \a factory provides. Prints a warning if a widget is 
  already supported. In this case createWidget() uses the factory added last.

  \sa widgetList()
*/
void QWidgetFactory::installWidgetFactory( QWidgetFactory* factory )
{
    QStringList widgets = factory->enumerateWidgets();
    for ( uint w = 0; w < widgets.count(); w++ ) {
	if ( factories["WIDGET_"+widgets[w]] && factories["WIDGET_"+widgets[w]] != factory )
	    qWarning("More than one factory provides %s", widgets[w].latin1() );
	factories.insert( "WIDGET_"+widgets[w], factory );
    }
    QStringList filetypes = factory->enumerateFileTypes();
    for ( uint f = 0; f < filetypes.count(); f++ ) {
	if ( factories["FILE_"+filetypes[f]] && factories["FILE_"+filetypes[f]] != factory )
	    qWarning("More than one factory supports %s", filetypes[f].latin1() );
	factories.insert( "FILE_"+filetypes[f], factory );
    }

    if ( factories.count() > prime[primeSize] ) {
	if ( ++primeSize < 6 )
	    factories.resize( prime[++primeSize] );
    }
}

/*!
  Removes a factory.
  All widgets and filetypes supported by \a factory are no longer available by
  createWidget()

  \sa installWidgetFactory()
*/
void QWidgetFactory::removeWidgetFactory( QWidgetFactory* factory )
{
    QDictIterator<QWidgetFactory> it( factories );

    while (it.current() ) {
	if ( it.current() == factory )
	    factories.remove( it.currentKey() );
	else
	    ++it;
    }
}

/*!
  Returns a list of installed factories

  \sa installWidgetFactory()
*/
QList<QWidgetFactory> QWidgetFactory::factoryList()
{
    QList<QWidgetFactory> list;

    QDictIterator<QWidgetFactory> it( factories );

    while ( it.current() ) {
	if ( !list.contains( it.current() ) )
	    list.append( it.current() );

	++it;
    }

    return list;
}

/*!
  Returns a list of names of all supported widgets.

  \sa installWidgetFactory(), enumerateWidgets()
*/
QStringList QWidgetFactory::widgetList()
{
    QStringList list;
    
    QDictIterator<QWidgetFactory> it( factories );

    while ( it.current() ) {
	QStringList widgets = it.current()->enumerateWidgets();
	for ( uint w = 0; w < widgets.count(); w++ ) {
	    if ( !list.contains( widgets[w] ) )
		list.append( widgets[w] );
	}

	++it;
    }
    
    return list;
}

/*!
  Returns the name of the factory that provides the widget \a classname.

  \sa installWidgetFactory()
*/
QString QWidgetFactory::widgetFactory( const QString& classname )
{
    QWidgetFactory* f = factories["WIDGET_"+classname];
    if ( f )
	return f->factoryName();
    else
	return "";
}

/*!
  Returns a widget of class \a classname.
  Looks up the widget factory that provides \a classname and creates
  the widget with \a parent, \a name and \a f.
  Returns 0 if the widget could not be created.

  \sa installWidgetFactory()
*/
QWidget* QWidgetFactory::createWidget( const QString& classname, QWidget* parent, const char* name, Qt::WFlags f )
{
    QWidgetFactory* fact = factories["WIDGET_"+classname];

    if ( fact )
	return fact->newWidget( classname, parent, name, f );
    return 0;
}

/*!
  Loads the file \a filename, creates and returns the widget if successful.
  Returns 0 if the widget could not be created.

  \sa processFile(), createWidget()
*/
QWidget* QWidgetFactory::createWidget( const QString &filename, bool &ok, QWidget *parent, const char *name, Qt::WFlags f )
{
    ok = FALSE;

    if ( filename.isEmpty() )
	return 0;

    QFile file( filename );
    if ( !file.open( IO_ReadOnly ) )
	return 0;

    QString fileext = "";
    int extpos = filename.findRev( '.' );
    if ( extpos != -1 )
	fileext = filename.right( filename.length() - extpos );

    QWidgetFactory* fact = factories["FILE_"+fileext];
    if ( fact ) {
	QWidget* w = fact->processFile( &file, ok );
	file.close();
	if ( w ) {
	    w->reparent( parent, f, w->pos() );
	    w->setName( name );
	}
	return w;
    }

    return 0;
}

/*!
  Processes the file \a f and returns a widget if successful.

  This method gets called by createWidget().
  Reimplement this function to add support for custom filetypes.
*/
QWidget* QWidgetFactory::processFile( QFile* f, bool &ok )
{
    ok = FALSE;

/*
    QDomDocument doc;
    if ( !doc.setContent( f ) ) {
	return 0;
    }
*/
    // TODO: process doc

    ok = TRUE;
    return 0;
}

/*!
  Returns a list of supported file types.

  Reimplement this function to add support for custom filetypes.
*/
QStringList QWidgetFactory::enumerateFileTypes()
{
    QStringList list;

    list << ".ui";

    return list;
}

/*!
  \fn QWidget* QWidgetFactory::newWidget( const QString& classname, QWidget* parent, const char* name, Qt::WFlags f )

  Creates and returns a widget registered with \a classname and passes \a parent, \a name
  and \a f to the widgets's constructor if successful. Otherwise returns 0.
  
  You have to reimplement this function in your factories to add support for custom widgets.
  Note that newWidget() is declared as private, so you musn't call the super-class.

  \sa enumerateWidgets()
*/

/*!
  \fn QStringList QWidgetFactory::enumerateWidgets()

  Returns a list of widget-classes supported by this factory.
  You have to reimplement this function in your factories to add support for custom widgets.
  Note that newWidget() is declared as private, so you musn't call the super-class.

  \sa newWidget()
*/

/*!
  \fn QString QWidgetFactory::factoryName() const

  Returns the name of the this factory.
  You have to reimplement this function in your factories.
*/

/*!
  \class QDefaultWidgetFactory qwidgetfactory.h

  \brief Provides support for standard Qt-widgets.
*/

/*!
  \reimp

  Note that some widget classes don't provide a constructor with a WFlags-parameter 
  in which case \a f is ignored silently.
*/
QWidget* QDefaultWidgetFactory::newWidget( const QString& classname, QWidget* parent, const char* name, Qt::WFlags f )
{
    QWidget* widget = 0;

    if ( classname == "QButtonGroup" ) {
	widget = new QButtonGroup( parent, name );
    } else if ( classname == "QCheckBox" ) {
	widget = new QCheckBox( parent, name );
    } else if ( classname == "QComboBox" ) {
	widget = new QComboBox( parent, name );
    } else if ( classname == "QDial" ) {
	widget = new QDial( parent, name );
    } else if ( classname == "QFrame" ) {
	widget = new QFrame( parent, name, f );
    } else if ( classname == "QGroupBox" ) {
	widget = new QGroupBox( parent, name );
    } else if ( classname == "QHBox" ) {
	widget = new QHBox( parent, name, f );
    } else if ( classname == "QHButtonGroup" ) {
	widget = new QHButtonGroup( parent, name );
    } else if ( classname == "QHeader" ) {
	widget = new QHeader( parent, name );
    } else if ( classname == "QHGroupBox" ) {
	widget = new QHGroupBox( parent, name );
    } else if ( classname == "QIconView" ) {
	widget = new QIconView( parent, name, f );
    } else if ( classname == "QLabel" ) {
	widget = new QLabel( parent, name, f );
    } else if ( classname == "QLCDNumber" ) {
	widget = new QLCDNumber( parent, name );
    } else if ( classname == "QLineEdit" ) {
	widget = new QLineEdit( parent, name );
    } else if ( classname == "QListBox" ) {
	widget = new QListBox( parent, name, f );
    } else if ( classname == "QListView" ) {
	widget = new QListView( parent, name, f );
    } else if ( classname == "QMainWindow" ) {
	widget = new QMainWindow( parent, name, f );
    } else if ( classname == "QMenuBar" ) {
	widget = new QMenuBar( parent, name );
    } else if ( classname == "QMultiLineEdit" ) {
	widget = new QMultiLineEdit( parent, name );
    } else if ( classname == "QPopupMenu" ) {
	widget = new QPopupMenu( parent, name );
    } else if ( classname == "QProgressBar" ) {
	widget = new QProgressBar( parent, name, f );
    } else if ( classname == "QPushButton" ) {
	widget = new QPushButton( parent, name );
    } else if ( classname == "QRadioButton" ) {
	widget = new QRadioButton( parent, name );
    } else if ( classname == "QScrollBar" ) {
	widget = new QScrollBar( parent, name );
    } else if ( classname == "QScrollView" ) {
	widget = new QScrollView( parent, name, f );
    } else if ( classname == "QSlider" ) {
	widget = new QSlider( parent, name );
    } else if ( classname == "QSpinBox" ) {
	widget = new QSpinBox( parent, name );
    } else if ( classname == "QSplitter" ) {
	widget = new QSplitter( parent, name );
    } else if ( classname == "QStatusBar" ) {
	widget = new QStatusBar( parent, name );
    } else if ( classname == "QTabBar" ) {
	widget = new QTabBar( parent, name );
    } else if ( classname == "QTabWidget" ) {
	widget = new QTabWidget( parent, name, f );
    } else if ( classname == "QTextBrowser" ) {
	widget = new QTextBrowser( parent, name );
    } else if ( classname == "QTextView" ) {
	widget = new QTextView( parent, name );
    } else if ( classname == "QToolBar" ) {
	if ( widget && widget->inherits( "QMainWindow" ) )
	    widget = new QToolBar( (QMainWindow*)parent, name );
    } else if ( classname == "QToolButton" ) {
	widget = new QToolButton( parent, name );
    } else if ( classname == "QVBox" ) {
	widget = new QVBox( parent, name, f );
    } else if ( classname == "QVButtonGroup" ) {
	widget = new QVButtonGroup( parent, name );
    } else if ( classname == "QVGroupBox" ) {
	widget = new QVGroupBox( parent, name );
    } else if ( classname == "QWidgetStack" ) {
	widget = new QWidgetStack( parent, name );
    } else if ( classname == "QWorkspace" ) {
	widget = new QWorkspace( parent, name );
    } else if ( classname == "QWidget" ) {
	widget = new QWidget( parent, name, f );
    } else {
	qWarning("Widget class %s not supported by QDefaultWidgetFactory!", classname.latin1() );
    }

    return widget;
}

/*!
  \reimp
*/
QStringList QDefaultWidgetFactory::enumerateWidgets()
{
    QStringList list;

    list << "QButtonGroup";
    list << "QCheckBox";
    list << "QComboBox";
    list << "QDial";
    list << "QFrame";
    list << "QGroupBox";
    list << "QHBox";
    list << "QHButtonGroup";
    list << "QHeader";
    list << "QHGroupBox";
    list << "QIconView";
    list << "QLabel";
    list << "QLCDNumber";
    list << "QLineEdit";
    list << "QListBox";
    list << "QListView";
    list << "QMainWindow";
    list << "QMenuBar";
    list << "QMultiLineEdit";
    list << "QPopupMenu";
    list << "QProgressBar";
    list << "QPushButton";    
    list << "QRadioButton";
    list << "QScrollBar";
    list << "QScrollView";
    list << "QSlider";
    list << "QSpinBox";
    list << "QSplitter";
    list << "QStatusBar";
    list << "QTabBar";
    list << "QTabWidget";
    list << "QTextBrowser";
    list << "QTextView";
    list << "QToolBar";
    list << "QToolButton";
    list << "QVBox";
    list << "QVButtonGroup";
    list << "QVGroupBox";
    list << "QWidgetStack";
    list << "QWorkspace";
    list << "QWidget";

    return list;
}

/*!
  \fn QString QDefaultWidgetFactory::factoryName() const
  \reimp
*/
