#include "qwidgetfactory.h"
#include <qfileinfo.h>

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
QDict<QWidgetFactory> QWidgetFactory::factories( prime[0] );

/*!
  \class QWidgetFactory qwidgetfactory.h
  \brief Factory-class for widgets.

  Normal use of this class is to call QWidgetFactory::create() with the
  name of a file or widget class, and get a QWidget* in return.

  As supplied, QWidgetFactory can create all the widgets in Qt, but it
  can be extended with support for custom widgets. To do that, you must
  subclass QWidgetFactory, reimplement newWidget() and make your
  reimplementation create the widget types requested. QWidgetFactory
  uses setProperties() to configure the widgets once they're created, so
  your custom widgets must support properties.
*/

/*!
  Installs a QWidgetFactory.
  Registers all widgets and filetypes \a factory provides.

  \sa widgetList(), fileTypeList()
*/
void QWidgetFactory::installWidgetFactory( QWidgetFactory* factory )
{
    QStringList widgets = factory->enumerateWidgets();
    for ( QStringList::Iterator w = widgets.begin(); w != widgets.end(); w++ ) {
#ifdef CHECK_RANGE
	if ( factories[*w] && factories[*w] != factory )
	    qWarning("More than one factory provides %s", (*w).latin1() );
#endif
	factories.insert( *w, factory );
    }
    QStringList filetypes = factory->enumerateFileTypes();
    for ( QStringList::Iterator f = filetypes.begin(); f != filetypes.end(); f++ ) {
#ifdef CHECK_RANGE
	if ( factories[*f] && factories[*f] != factory )
	    qWarning("More than one factory supports %s", (*f).latin1() );
#endif
	factories.insert( *f, factory );
    }

    if ( factories.count() > prime[primeSize] ) {
	if ( primeSize <= 6 )
	    factories.resize( prime[++primeSize] );
    }
}

/*!
  Removes a factory.
  All widgets and filetypes supported by \a factory are no longer available. 

  \sa installWidgetFactory(), createWidget()
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
    QStringList l;
    
    QList<QWidgetFactory> list = factoryList();
    QListIterator<QWidgetFactory> it( list );

    while ( it.current() ) {
	QStringList widgets = it.current()->enumerateWidgets();
	for ( QStringList::Iterator w = widgets.begin(); w != widgets.end(); w++ ) {
	    if ( !l.contains( *w ) )
		l.append( *w );
	}
	++it;
    }
    
    return l;
}

/*!
  Returns a list of supported filetypes
*/
QStringList QWidgetFactory::fileTypeList()
{
    QStringList l;
    QList<QWidgetFactory> list = factoryList();
    QListIterator<QWidgetFactory> it( list );

    while ( it.current() ) {
	QStringList types = it.current()->enumerateFileTypes();
	for ( QStringList::Iterator lt = types.begin(); lt != types.end() ; ++lt ) {
	    if ( !l.contains( *lt ) )
		l.append( *lt );
	}
	++it;
    }

    return l;
}

/*!
  Returns the name of the factory that provides the widget \a classname.

  \sa installWidgetFactory()
*/
QString QWidgetFactory::widgetFactory( const QString& classname )
{
    QWidgetFactory* f = factories[classname];
    if ( f )
	return f->factoryName();
    else
	return "";
}

/*!
  Returns a widget of class \a classname.
  Looks up the widget factory that provides \a classname and creates
  the widget with \a parent and \a name. If \a init is TRUE the widget
  gets initialized by the factory.
  Returns 0 if the widget could not be created.

  \sa installWidgetFactory()
*/
QWidget* QWidgetFactory::createWidget( const QString& classname, bool init, QWidget* parent, const char* name )
{
    QWidgetFactory* fact = factories[classname];

    if ( fact )
	return fact->newWidget( classname, init, parent, name );
    return 0;
}

/*!
  Loads the file \a filename and calls processFile() to create the widget.
  Returns the widget if successful or 0 if the widget could not be created.

  \sa createWidget()
*/
QWidget* QWidgetFactory::createWidget( const QString &filename, QWidget *parent, const char *name, Qt::WFlags f )
{
    if ( filename.isEmpty() )
	return 0;

    QFile file( filename );
    if ( !file.open( IO_ReadOnly ) )
	return 0;

    QFileInfo fi( file );

    QDictIterator<QWidgetFactory> it( factories );
    QWidgetFactory* fact = 0;
    QString type;
    while ( it.current() && !fact ) {
	QStringList types = it.current()->enumerateFileTypes();
	for ( QStringList::Iterator t = types.begin(); t != types.end(); t++ ) {
	    QString ext = *t;
	    QRegExp r( QString::fromLatin1("([a-zA-Z0-9.*? +;#]*)$") );
	    int len;
	    int index = r.match( ext, 0, &len );
	    if ( index >= 0 )
		ext = ext.mid( index+1, len-2 );

	    QStringList extensions = QStringList::split( QRegExp("[;\\s]"), ext );
	    for ( QStringList::Iterator e = extensions.begin(); e != extensions.end() && !fact; e++ ) {
		ext = *e;
		ext.replace( QRegExp("[*]?[.]"), "" );
		if ( ext == fi.extension() ) {
		    fact = it.current();
		    type = *t;
		    continue;
		}
	    }
	}

	++it;
    }

    if ( fact ) {
	QWidget* w = fact->processFile( &file, type );
	file.close();
	if ( w ) {
	    if ( parent || f )
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
QWidget* QWidgetFactory::processFile( QIODevice* f, const QString& filetype )
{
    qDebug("Imagine I process %s", filetype.latin1() );

    if ( filetype.contains( "*.ui" ) ) {
    } else if ( filetype.contains( "*.pro" ) ) {
/*	QStringList lst = getUIFiles( f );
	for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
	    QString fn = QUrl( QFileInfo( filename ).dirPath(), *it ).path();
	    openFile( fn );
	}
*/    }

/*
    QDomDocument doc;
    if ( !doc.setContent( f ) ) {
	return 0;
    }
*/
    // TODO: process doc

    return 0;
}

/*!
  Returns a list of supported file types.

  Reimplement this function to add support for custom filetypes.
*/
QStringList QWidgetFactory::enumerateFileTypes()
{
    QStringList list;

    list << "Qt User Interface File (*.ui)";
    list << "TMAKE Projectfile (*.pro)";

    return list;
}

/*!
  Creates and returns a widget registered with \a classname and passes \a parent and \a name
  to the widgets's constructor if successful. Otherwise returns 0.
  The widget gets initialized with by the factory if \a init is TRUE.
  
  You have to reimplement this function in your factories to add support for custom widgets.
  Note that newWidget() is declared as private, so you musn't call the super-class.

  \sa enumerateWidgets()
*/
QWidget* QWidgetFactory::newWidget( const QString& classname, bool init, QWidget* parent, const char* name )
{
    if ( classname == "QButtonGroup" ) {
	return init ? new QButtonGroup( QString(name), parent, name ) : 
		      new QButtonGroup( parent, name );
    } else if ( classname == "QCheckBox" ) {
	return init ? new QCheckBox( QString(name), parent, name ) :
		      new QCheckBox( parent, name );
    } else if ( classname == "QComboBox" ) {
	return new QComboBox( FALSE, parent, name );
    } else if ( classname == "QDial" ) {
	return new QDial( parent, name );
    } else if ( classname == "QFrame" ) {
	QFrame *widget = new QFrame( parent, name );
	if ( init )
	    widget->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
	return widget;
    } else if ( classname == "QGroupBox" ) {
	return new QGroupBox( parent, name );
    } else if ( classname == "QHBox" ) {
	return new QHBox( parent, name );
    } else if ( classname == "QHButtonGroup" ) {
	return init ? new QHButtonGroup( QString(name), parent, name ) :
		      new QHButtonGroup( parent, name );
    } else if ( classname == "QHeader" ) {
	return new QHeader( parent, name );
    } else if ( classname == "QHGroupBox" ) {
	return init ? new QHGroupBox( QString(name), parent, name ) :
		      new QHGroupBox( parent, name );
    } else if ( classname == "QIconView" ) {
	return new QIconView( parent, name );
    } else if ( classname == "QLabel" ) {
	return init ? new QLabel( QString(name), parent, name ) :
		      new QLabel( parent, name );
    } else if ( classname == "QLCDNumber" ) {
	return new QLCDNumber( parent, name );
    } else if ( classname == "QLineEdit" ) {
	return new QLineEdit( parent, name );
    } else if ( classname == "QListBox" ) {
	return new QListBox( parent, name );
    } else if ( classname == "QListView" ) {
	QListView *widget = new QListView( parent, name );
	if ( init )
	    widget->addColumn( "Column 1" );
	return widget;
    } else if ( classname == "QMainWindow" ) {
	return new QMainWindow( parent, name );
    } else if ( classname == "QMenuBar" ) {
	return new QMenuBar( parent, name );
    } else if ( classname == "QMultiLineEdit" ) {
	return new QMultiLineEdit( parent, name );
    } else if ( classname == "QPopupMenu" ) {
	return new QPopupMenu( parent, name );
    } else if ( classname == "QProgressBar" ) {
	return new QProgressBar( parent, name );
    } else if ( classname == "QPushButton" ) {
	return init ? new QPushButton( QString(name), parent, name ) :
		      new QPushButton( parent, name );
    } else if ( classname == "QRadioButton" ) {
	return init ? new QRadioButton( QString(name), parent, name ) :
		      new QRadioButton( parent, name );
    } else if ( classname == "QScrollBar" ) {
	return new QScrollBar( parent, name );
    } else if ( classname == "QScrollView" ) {
	return new QScrollView( parent, name );
    } else if ( classname == "QSlider" ) {
	return new QSlider( parent, name );
    } else if ( classname == "QSpinBox" ) {
	return new QSpinBox( parent, name );
    } else if ( classname == "QSplitter" ) {
	return new QSplitter( parent, name );
    } else if ( classname == "QStatusBar" ) {
	return new QStatusBar( parent, name );
    } else if ( classname == "QTabBar" ) {
	return new QTabBar( parent, name );
    } else if ( classname == "QTabWidget" ) {
	QTabWidget *widget = new QTabWidget( parent, name );
	return widget;
    } else if ( classname == "QTextBrowser" ) {
	return new QTextBrowser( parent, name );
    } else if ( classname == "QTextView" ) {
	return new QTextView( parent, name );
    } else if ( classname == "QToolBar" ) {
	if (  parent &&  parent->inherits( "QMainWindow" ) ) {
	    QToolBar *widget = new QToolBar( (QMainWindow*)parent, name );
	    return widget;
	}
    } else if ( classname == "QToolButton" ) {
	QToolButton *widget = new QToolButton( parent, name );
	if ( init )
	    widget->setText( "..." );
	return widget;
    } else if ( classname == "QVBox" ) {
	return new QVBox( parent, name );
    } else if ( classname == "QVButtonGroup" ) {
	return init ? new QVButtonGroup( QString(name), parent, name ) :
		      new QVButtonGroup( parent, name );
    } else if ( classname == "QVGroupBox" ) {
	return init ? new QVGroupBox( QString(name), parent, name ) :
		      new QVGroupBox( parent, name );
    } else if ( classname == "QWidgetStack" ) {
	return new QWidgetStack( parent, name );
    } else if ( classname == "QWorkspace" ) {
	return new QWorkspace( parent, name );
    } else if ( classname == "QWidget" ) {
	return new QWidget( parent, name );
    } else {
	qWarning("Widget class %s not supported by QWidgetFactory!", classname.latin1() );
    }

    return 0;
}

/*!
  Returns a list of widget-classes supported by this factory.
  You have to reimplement this function in your factories to add support for custom widgets.
  Note that newWidget() is declared as private, so you musn't call the super-class.

  \sa newWidget()
*/
QStringList QWidgetFactory::enumerateWidgets()
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
  \fn QString QWidgetFactory::factoryName() const

  Returns the name of the this factory.
  You have to reimplement this function in your factories.
*/

