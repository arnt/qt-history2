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
QDict<QWidgetFactory> QWidgetFactory::factory( prime[0] );
QList<QWidgetFactory> QWidgetFactory::factories = QList<QWidgetFactory>();
QWidgetFactory* QWidgetFactory::that = 0;

/*!
  \class QWidgetFactory qwidgetfactory.h
  \brief Factory-class for widgets.

  Normal use of this class is to call QWidgetFactory::create() with a
  widget description, and get a QWidget* in return.

  As supplied, QWidgetFactory can load ui-files and create all the widgets
  in Qt, but it can be extended with support for custom widgets. To do that,
  you must subclass QWidgetFactory, reimplement newWidget() and widgets() and
  make your reimplementation create the widget types requested. QWidgetFactory
  uses setProperties() to configure the widgets once they're created, so
  your custom widgets must support properties.
*/

/*!
  Installs a QWidgetFactory.
  Registers all widgets \a factory provides.

  \sa widgetList()
*/
void QWidgetFactory::installWidgetFactory( QWidgetFactory* f )
{
    if ( !that )
	that = f;

    if ( !factories.contains( f ) )
	factories.append( f );

    QStringList widgets = f->widgets();
    for ( QStringList::Iterator w = widgets.begin(); w != widgets.end(); w++ ) {
#ifdef CHECK_RANGE
	if ( factory[*w] && factory[*w] != f )
	    qWarning("More than one factory provides %s", (*w).latin1() );
#endif
	factory.insert( *w, f );
    }

    if ( factory.count() > prime[primeSize] ) {
	if ( primeSize <= 6 )
	    factory.resize( prime[++primeSize] );
    }
}

/*!
  Removes the widget factory \a f.
  All widgets supported by \a f are no longer available.

  \sa installWidgetFactory(), createWidget()
*/
void QWidgetFactory::removeWidgetFactory( QWidgetFactory* f )
{
    factories.remove( f );
    QDictIterator<QWidgetFactory> it( factory );

    while (it.current() ) {
	if ( it.current() == f )
	    factory.remove( it.currentKey() );
	else
	    ++it;
    }
}

/*!
  Returns the list of installed factories

  \sa installWidgetFactory()
*/
QList<QWidgetFactory> QWidgetFactory::factoryList()
{
    return factories;
}

/*!
  Returns a list of names of all supported widgets.

  \sa installWidgetFactory(), widgets()
*/
QStringList QWidgetFactory::widgetList()
{
    QStringList l;

    QList<QWidgetFactory> list = factories;
    QListIterator<QWidgetFactory> it( list );

    while ( it.current() ) {
	QStringList widgets = it.current()->widgets();
	for ( QStringList::Iterator w = widgets.begin(); w != widgets.end(); w++ ) {
	    if ( !l.contains( *w ) )
		l.append( *w );
	}
	++it;
    }

    return l;
}

/*!
  Returns the name of the factory that provides the widget \a classname.

  \sa installWidgetFactory()
*/
QWidgetFactory *QWidgetFactory::widgetFactory( const QString& classname )
{
   return factory[classname];
}

/*!
  Returns a widget that matches \a description.

  Tries to process a UI-description and returns the created widget
  if successful, or looks up the widget factory that provides a widget
  matching the description and creates the widget with \a parent and
  \a name.
  Returns 0 if the widget could not be created.

  \sa installWidgetFactory()
*/
QWidget* QWidgetFactory::create( const QString& description, QWidget* parent, const char* name )
{
    if ( description.isEmpty() )
	return 0;

    if ( description[0] == '<' ) {
	QWidget* w = 0;
	if ( !that )
	    installWidgetFactory( new QWidgetFactory );
	w = that->compose( description );

	if ( w ) {
	    if ( parent )
		w->reparent( parent, w->pos() );
	    w->setName( name );
	}
	return w;
    }

    QWidgetFactory* fact = factory[description];
    if ( !fact ) {
	QListIterator<QWidgetFactory> it( factories );
	while ( it.current() ) {
	    if ( it.current()->widgets().contains( description ) ) {
		factory.insert( description, it.current() );
		fact = it.current();
		break;
	    }
	    ++it;
	}
    }

    if ( fact )
	return fact->newWidget( description, parent, name );
    return 0;
}

/*!
  Processes \a description and returns a widget if successful.
*/
QWidget* QWidgetFactory::compose( const QString& description )
{
    qDebug("Imagine I process %s", description.latin1() );

    // TODO: process document

    return 0;
}

/*!
  Creates and returns a widget registered with \a description and passes \a parent and \a name
  to the widgets's constructor if successful. Otherwise returns 0.

  You have to reimplement this function in your factories to add support for custom widgets.
  Note that newWidget() is declared as private, so you musn't call the super-class.

  \sa enumerateWidgets()
*/
QWidget* QWidgetFactory::newWidget( const QString& description, QWidget* parent, const char* name )
{
    if ( description == "QButtonGroup" ) {
	return new QButtonGroup( parent, name );
    } else if ( description == "QCheckBox" ) {
	return new QCheckBox( parent, name );
    } else if ( description == "QComboBox" ) {
	return new QComboBox( FALSE, parent, name );
    } else if ( description == "QDial" ) {
	return new QDial( parent, name );
    } else if ( description == "QFrame" ) {
	return new QFrame( parent, name );
    } else if ( description == "QGroupBox" ) {
	return new QGroupBox( parent, name );
    } else if ( description == "QHBox" ) {
	return new QHBox( parent, name );
    } else if ( description == "QHButtonGroup" ) {
	return new QHButtonGroup( parent, name );
    } else if ( description == "QHeader" ) {
	return new QHeader( parent, name );
    } else if ( description == "QHGroupBox" ) {
	return new QHGroupBox( parent, name );
    } else if ( description == "QIconView" ) {
	return new QIconView( parent, name );
    } else if ( description == "QLabel" ) {
	return new QLabel( parent, name );
    } else if ( description == "QLCDNumber" ) {
	return new QLCDNumber( parent, name );
    } else if ( description == "QLineEdit" ) {
	return new QLineEdit( parent, name );
    } else if ( description == "QListBox" ) {
	return new QListBox( parent, name );
    } else if ( description == "QListView" ) {
	return new QListView( parent, name );
    } else if ( description == "QMainWindow" ) {
	return new QMainWindow( parent, name );
    } else if ( description == "QMenuBar" ) {
	return new QMenuBar( parent, name );
    } else if ( description == "QMultiLineEdit" ) {
	return new QMultiLineEdit( parent, name );
    } else if ( description == "QPopupMenu" ) {
	return new QPopupMenu( parent, name );
    } else if ( description == "QProgressBar" ) {
	return new QProgressBar( parent, name );
    } else if ( description == "QPushButton" ) {
	return new QPushButton( parent, name );
    } else if ( description == "QRadioButton" ) {
	return new QRadioButton( parent, name );
    } else if ( description == "QScrollBar" ) {
	return new QScrollBar( parent, name );
    } else if ( description == "QScrollView" ) {
	return new QScrollView( parent, name );
    } else if ( description == "QSlider" ) {
	return new QSlider( parent, name );
    } else if ( description == "QSpinBox" ) {
	return new QSpinBox( parent, name );
    } else if ( description == "QSplitter" ) {
	return new QSplitter( parent, name );
    } else if ( description == "QStatusBar" ) {
	return new QStatusBar( parent, name );
    } else if ( description == "QTabBar" ) {
	return new QTabBar( parent, name );
    } else if ( description == "QTabWidget" ) {
	return new QTabWidget( parent, name );
    } else if ( description == "QTextBrowser" ) {
	return new QTextBrowser( parent, name );
    } else if ( description == "QTextView" ) {
	return new QTextView( parent, name );
    } else if ( description == "QToolBar" ) {
	if (  parent &&  parent->inherits( "QMainWindow" ) )
	    return new QToolBar( (QMainWindow*)parent, name );
#ifdef CHECK_RANGE
	else
	    qWarning( "QToolBar needs a QMainWindow derived class as parent!" );
#endif
    } else if ( description == "QToolButton" ) {
	return new QToolButton( parent, name );
    } else if ( description == "QVBox" ) {
	return new QVBox( parent, name );
    } else if ( description == "QVButtonGroup" ) {
	return new QVButtonGroup( parent, name );
    } else if ( description == "QVGroupBox" ) {
	return new QVGroupBox( parent, name );
    } else if ( description == "QWidgetStack" ) {
	return new QWidgetStack( parent, name );
    } else if ( description == "QWorkspace" ) {
	return new QWorkspace( parent, name );
    } else if ( description == "QWidget" ) {
	return new QWidget( parent, name );
    } else {
	qWarning("Widget class %s not supported by QWidgetFactory!", description.latin1() );
    }

    return 0;
}

/*!
  Returns a list of widget-description, e.g. classnames, supported by this factory.

  You have to reimplement this function in your factories to add support for custom widgets.
  Note that newWidget() is declared as private, so you musn't call the super-class.

  \sa newWidget()
*/
QStringList QWidgetFactory::widgets()
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

