#include "../../../../../tools/designer/shared/widgetinterface.h"

#include <qapplication.h>
#include <qcleanuphandler.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdial.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qheader.h>
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
#include <qwidgetstack.h>

#ifdef _WS_WIN_
#undef LIBEXPORT
#define LIBEXPORT __declspec(dllexport)
#else
#define LIBEXPORT
#endif

class QWidgetsInterface : public WidgetInterface
{
public:
    QWidgetsInterface();
    ~QWidgetsInterface();

    bool connectNotify( QApplication* theApp );
    bool disconnectNotify( QApplication* theApp );

    QString name() { return "Standard Widgets"; }
    QString description() { return "Qt Designer plugin for standard widgets"; }
    QString author() { return "Trolltech"; }

    QStringList featureList();
    QWidget* create( const QString &classname, QWidget* parent = 0, const char* name = 0 );
    QString group( const QString& );
    QString iconSet( const QString& );
    QString includeFile( const QString& );
    QString toolTip( const QString& );
    QString whatsThis( const QString& );
    bool isContainer( const QString& );

    QGuardedCleanUpHandler<QObject> objects;
};

QWidgetsInterface::QWidgetsInterface()
{
}

QWidgetsInterface::~QWidgetsInterface()
{
}

bool QWidgetsInterface::connectNotify( QApplication* theApp )
{
    return TRUE;
}

bool QWidgetsInterface::disconnectNotify( QApplication* theApp )
{
    if ( !objects.clean() )
	return FALSE;
    return TRUE;
}

QStringList QWidgetsInterface::featureList()
{
    QStringList list;

    list << "QButtonGroup";
    list << "QCheckBox";
    list << "QComboBox";
    list << "QDial";
    list << "QFrame";
    list << "QGroupBox";
    list << "QHeader";
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
    list << "QWidgetStack";
    list << "QWidget";    

    return list;
}

QWidget* QWidgetsInterface::create( const QString &description, QWidget* parent, const char* name )
{
    QWidget* w = 0;
    if ( description == "QButtonGroup" ) {
	w = new QButtonGroup( parent, name );
    } else if ( description == "QCheckBox" ) {
	w = new QCheckBox( parent, name );
    } else if ( description == "QComboBox" ) {
	w = new QComboBox( FALSE, parent, name );
    } else if ( description == "QDial" ) {
	w = new QDial( parent, name );
    } else if ( description == "QFrame" ) {
	w = new QFrame( parent, name );
    } else if ( description == "QGroupBox" ) {
	w = new QGroupBox( parent, name );
    } else if ( description == "QHeader" ) {
	w = new QHeader( parent, name );
    } else if ( description == "QLabel" ) {
	w = new QLabel( parent, name );
    } else if ( description == "QLCDNumber" ) {
	w = new QLCDNumber( parent, name );
    } else if ( description == "QLineEdit" ) {
	w = new QLineEdit( parent, name );
    } else if ( description == "QListBox" ) {
	w = new QListBox( parent, name );
    } else if ( description == "QListView" ) {
	w = new QListView( parent, name );
    } else if ( description == "QMainWindow" ) {
	w = new QMainWindow( parent, name );
    } else if ( description == "QMenuBar" ) {
	w = new QMenuBar( parent, name );
    } else if ( description == "QMultiLineEdit" ) {
	w = new QMultiLineEdit( parent, name );
    } else if ( description == "QPopupMenu" ) {
	w = new QPopupMenu( parent, name );
    } else if ( description == "QProgressBar" ) {
	w = new QProgressBar( parent, name );
    } else if ( description == "QPushButton" ) {
	w = new QPushButton( parent, name );
    } else if ( description == "QRadioButton" ) {
	w = new QRadioButton( parent, name );
    } else if ( description == "QScrollBar" ) {
	w = new QScrollBar( parent, name );
    } else if ( description == "QScrollView" ) {
	w = new QScrollView( parent, name );
    } else if ( description == "QSlider" ) {
	w = new QSlider( parent, name );
    } else if ( description == "QSpinBox" ) {
	w = new QSpinBox( parent, name );
    } else if ( description == "QSplitter" ) {
	w = new QSplitter( parent, name );
    } else if ( description == "QStatusBar" ) {
	w = new QStatusBar( parent, name );
    } else if ( description == "QTabBar" ) {
	w = new QTabBar( parent, name );
    } else if ( description == "QTabWidget" ) {
	w = new QTabWidget( parent, name );
    } else if ( description == "QTextBrowser" ) {
	w = new QTextBrowser( parent, name );
    } else if ( description == "QTextView" ) {
	w = new QTextView( parent, name );
    } else if ( description == "QToolBar" ) {
	if (  parent &&  parent->inherits( "QMainWindow" ) )
	    w = new QToolBar( (QMainWindow*)parent, name );
#ifdef CHECK_RANGE
	else
	    qWarning( "QToolBar needs a QMainWindow derived class as parent!" );
#endif
    } else if ( description == "QToolButton" ) {
	w = new QToolButton( parent, name );
    } else if ( description == "QWidgetStack" ) {
	w = new QWidgetStack( parent, name );
    } else if ( description == "QWidget" ) {
	w = new QWidget( parent, name );
#ifdef CHECK_RANGE
    } else {
	qWarning("Widget class %s not supported by QWidgetFactory!", description.latin1() );
#endif
    }

    objects.addCleanUp( w );
    return w;
}

QString QWidgetsInterface::group( const QString& description )
{
    if ( description == "QButtonGroup" )
	return "Containers";
    else if ( description == "QCheckBox" )
	return "Buttons";
    else if ( description == "QComboBox" )
	return "Input";
    else if ( description == "QDial" )
	return "Input";
    else if ( description == "QFrame" )
	return "Containers";
    else if ( description == "QGroupBox" )
	return "Containers";
    else if ( description == "QHeader" )
	return "Decoration";
    else if ( description == "QLabel" )
	return "Display";
    else if ( description == "QLCDNumber" )
	return "Display";
    else if ( description == "QLineEdit" )
	return "Input";
    else if ( description == "QListBox" )
	return "Views";
    else if ( description == "QListView" )
	return "Views";
    else if ( description == "QMainWindow" )
	return "Extended";
    else if ( description == "QMenuBar" )
	return "Control";
    else if ( description == "QMultiLineEdit" )
	return "Input";
    else if ( description == "QPopupMenu" )
	return "Control";
    else if ( description == "QProgressBar" )
	return "Display";
    else if ( description == "QPushButton" )
	return "Buttons";
    else if ( description == "QRadioButton" )
	return "Buttons";
    else if ( description == "QScrollBar" )
	return "Decoration";
    else if ( description == "QScrollView" )
	return "Containers";
    else if ( description == "QSlider" )
	return "Input";
    else if ( description == "QSpinBox" )
	return "Input";
    else if ( description == "QSplitter" )
	return "Containers";
    else if ( description == "QStatusBar" )
	return "Display";
    else if ( description == "QTabBar" )
	return "Decoration";
    else if ( description == "QTabWidget" )
	return "Containers";
    else if ( description == "QTextBrowser" )
	return "Views";
    else if ( description == "QTextView" )
	return "Views";
    else if ( description == "QToolBar" )
	return "Containers";
    else if ( description == "QToolButton" )
	return "Buttons";
    else if ( description == "QWidgetStack" )
	return "Containers";
    else if ( description == "QWidget" )
	return "Containers";
	
    return QString::null;
}

QString QWidgetsInterface::iconSet( const QString& )
{
    return QString::null;
}

QString QWidgetsInterface::includeFile( const QString& description )
{
    if ( description == "QButtonGroup" )
	return "Containers";
    else if ( description == "QCheckBox" )
	return "Buttons";
    else if ( description == "QComboBox" )
	return "Input";
    else if ( description == "QDial" )
	return "Input";
    else if ( description == "QFrame" )
	return "Containers";
    else if ( description == "QGroupBox" )
	return "Containers";
    else if ( description == "QHeader" )
	return "Decoration";
    else if ( description == "QLabel" )
	return "Display";
    else if ( description == "QLCDNumber" )
	return "Display";
    else if ( description == "QLineEdit" )
	return "Input";
    else if ( description == "QListBox" )
	return "Views";
    else if ( description == "QListView" )
	return "Views";
    else if ( description == "QMainWindow" )
	return "Extended";
    else if ( description == "QMenuBar" )
	return "Control";
    else if ( description == "QMultiLineEdit" )
	return "Input";
    else if ( description == "QPopupMenu" )
	return "Control";
    else if ( description == "QProgressBar" )
	return "Display";
    else if ( description == "QPushButton" )
	return "Buttons";
    else if ( description == "QRadioButton" )
	return "Buttons";
    else if ( description == "QScrollBar" )
	return "Decoration";
    else if ( description == "QScrollView" )
	return "Containers";
    else if ( description == "QSlider" )
	return "Input";
    else if ( description == "QSpinBox" )
	return "Input";
    else if ( description == "QSplitter" )
	return "Containers";
    else if ( description == "QStatusBar" )
	return "Display";
    else if ( description == "QTabBar" )
	return "Decoration";
    else if ( description == "QTabWidget" )
	return "Containers";
    else if ( description == "QTextBrowser" )
	return "Views";
    else if ( description == "QTextView" )
	return "Views";
    else if ( description == "QToolBar" )
	return "Containers";
    else if ( description == "QToolButton" )
	return "Buttons";
    else if ( description == "QWidgetStack" )
	return "Containers";
    else if ( description == "QWidget" )
	return "Containers";
    
    return QString::null;
}

QString QWidgetsInterface::toolTip( const QString& description )
{
    if ( description == "QButtonGroup" )
	return "Button Group";
    else if ( description == "QCheckBox" )
	return "Check Box";
    else if ( description == "QComboBox" )
	return "Combo Box";
    else if ( description == "QDial" )
	return "Dial";
    else if ( description == "QFrame" )
	return "Frame";
    else if ( description == "QGroupBox" )
	return "Group Box";
    else if ( description == "QHeader" )
	return "Header";
    else if ( description == "QLabel" )
	return "Label";
    else if ( description == "QLCDNumber" )
	return "LCD Number";
    else if ( description == "QLineEdit" )
	return "Line Edit";
    else if ( description == "QListBox" )
	return "List Box";
    else if ( description == "QListView" )
	return "List View";
    else if ( description == "QMainWindow" )
	return "Main Window";
    else if ( description == "QMenuBar" )
	return "Menu Bar";
    else if ( description == "QMultiLineEdit" )
	return "Multi Line Edit";
    else if ( description == "QPopupMenu" )
	return "Popup Menu";
    else if ( description == "QProgressBar" )
	return "Progress Bar";
    else if ( description == "QPushButton" )
	return "Push Button";
    else if ( description == "QRadioButton" )
	return "Radio Button";
    else if ( description == "QScrollBar" )
	return "Scroll Bar";
    else if ( description == "QScrollView" )
	return "Scroll View";
    else if ( description == "QSlider" )
	return "Slider";
    else if ( description == "QSpinBox" )
	return "Spin Box";
    else if ( description == "QSplitter" )
	return "Splitter";
    else if ( description == "QStatusBar" )
	return "Status Bar";
    else if ( description == "QTabBar" )
	return "Tab Bar";
    else if ( description == "QTabWidget" )
	return "Tab Widget";
    else if ( description == "QTextBrowser" )
	return "Text Browser";
    else if ( description == "QTextView" )
	return "Text View";
    else if ( description == "QToolBar" )
	return "Tool Bar";
    else if ( description == "QToolButton" )
	return "Tool Button";
    else if ( description == "QWidgetStack" )
	return "Widget Stack";
    else if ( description == "QWidget" )
	return "Widget";

    return QString::null;
}

QString QWidgetsInterface::whatsThis( const QString& )
{
    return QString::null;
}

bool QWidgetsInterface::isContainer( const QString& description)
{
    if ( ( description == "QButtonGroup" ) ||
    ( description == "QFrame" ) ||
    ( description == "QGroupBox" ) ||
    ( description == "QMainWindow" ) ||
    ( description == "QScrollView" ) ||
    ( description == "QSplitter" ) ||
    ( description == "QTabWidget" ) ||
    ( description == "QToolBar" ) ||
    ( description == "QWidgetStack" ) ||
    ( description == "QWidget" ) )
	return TRUE;
    
    return FALSE;
}

#if defined(__cplusplus )
extern "C"
{
#endif

LIBEXPORT WidgetInterface* loadInterface()
{
    return new QWidgetsInterface();
}

#if defined(__cplusplus)
}
#endif // __cplusplus
