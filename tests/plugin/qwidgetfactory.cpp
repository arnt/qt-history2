#include "qwidgetfactory.h"
#include <qdict.h>

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
int primeSize = 0;
QDict<QWidgetFactory> factories( prime[0] );

QWidget* QWidgetFactory::createWidget( const QString& classname, QWidget* parent, const char* name, Qt::WFlags f )
{
    QWidgetFactory* fact = factories[classname];

    if ( fact )
	return fact->newWidget( classname, parent, name, f );
    return 0;
}

void QWidgetFactory::installWidgetFactory( QWidgetFactory* factory )
{
    QStringList widgets = factory->enumerateWidgets();
    for ( uint w = 0; w < widgets.count(); w++ ) {
	if ( factories[widgets[w]] && factories[widgets[w]] != factory )
	    qWarning("More than one factory creating %s", widgets[w].latin1() );
	factories.insert( widgets[w], factory );
    }
    if ( factories.count() > prime[primeSize] ) {
	if ( ++primeSize < 6 )
	    factories.resize( prime[++primeSize] );
    }
}

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

QString QWidgetFactory::widgetFactory( const QString& classname )
{
    QWidgetFactory* f = factories[classname];
    if ( f )
	return f->factoryName();
    else
	return "";
}

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