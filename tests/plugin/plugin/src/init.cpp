#include "previewstack.h"
#include "styledbutton.h"
#include "../../qwidgetinterface.h"
#include <qapplication.h>

#include <qworkspace.h>
#include <qscrollview.h>
#include <qtable.h>
#include <qiconview.h>

#ifdef _WS_WIN_
#undef LIBEXPORT
#define LIBEXPORT __declspec(dllexport)
#else
#define LIBEXPORT
#endif

static QString* resource = 0;

static void clean_me_up()
{
    delete resource;
    resource = 0;
}

class TestInterface : public QWidgetInterface
{
public:
    TestInterface() 
    {
	resource = new QString( "Ich bin der String!" );
	cleanUpHandler.addCleanUpRoutine( clean_me_up );
    }
    QString queryInterface() { return "QWidgetInterface"; }

    QString name() { return "Test Widgetplugin"; }
    QString description() { return "Test implementation of the QWidgetInterface"; }
    QString author() { return "vohi"; }

    QStringList widgets();
    QWidget* create( const QString &classname, QWidget* parent = 0, const char* name = 0 );
/*    QString iconSet( const QString &classname );
    QCString includeFile( const QString &classname );
    QString group( const QString &classname );
    QString toolTip( const QString &classname );
    QString whatsThis( const QString &classname );
    bool isContainer( const QString &classname );*/
};

QStringList TestInterface::widgets()
{
    QStringList w;

    w << "StyledButton";
    w << "MyTable";
    w << "MyScrollView";
    w << "MyWorkspace";
    w << "MyIconView";
    
    return w;
}

QWidget* TestInterface::create( const QString &classname, QWidget* parent, const char* name )
{
    if ( classname == "StyledButton" )
	return new StyledButton( parent, name );
    else if ( classname == "MyTable" )
	return new QTable( parent, name );
    else if ( classname == "MyScrollView" )
	return new QScrollView( parent, name );
    else if ( classname == "MyWorkspace" )
	return new QWorkspace( parent, name );
    else if ( classname == "MyIconView" )
	return new QIconView( parent, name );
    else
	return 0;
}

/*
QString TestInterface::iconSet( const QString& classname )
{
    if ( classname == "StyledButton" )
	return "/home/reggie/uparrow.xbm";
    else if ( classname == "QTable" )
	return "/home/reggie/uparrow.xbm";
    else 
	return QString();
}

QCString TestInterface::includeFile( const QString &classname )
{
    if ( classname == "StyledButton" )
	return "styledbutton.h";
    else if ( classname == "MyScrollView" )
	return "qscrollview.h";
    else if ( classname == "MyWorkspace" )
	return "qworkspace.h";
    return QCString("Bla");
}

QString TestInterface::group( const QString &classname )
{
    if ( classname == "StyledButton" )
	return "Buttons";
    else if ( classname == "MyScrollView" )
	return "Managers";
    else if ( classname == "MyWorkspace" )
	return "Managers";
    return QDefaultInterface::group( classname );
}

QString TestInterface::toolTip( const QString &classname )
{
    return QDefaultInterface::toolTip( classname );
}

QString TestInterface::whatsThis( const QString &classname )
{
    if ( classname == "StyledButton" )
	return "A StyledButton displays a color or pixmap and opens the appropriate dialog.";
    return QDefaultInterface::whatsThis( classname );
}

bool TestInterface::isContainer( const QString &classname )
{
    if( classname == "MyScrollView" )
	return TRUE;
    else if ( classname == "MyWorkspace" )
	return TRUE;
    else
	return FALSE;
}
*/
#if defined(__cplusplus )
extern "C"
{
#endif

LIBEXPORT QWidgetInterface* loadInterface()
{
    return new TestInterface();
}

LIBEXPORT bool onConnect()
{
    qDebug("I've been loaded!");
    return TRUE;
}

LIBEXPORT bool onDisconnect()
{
    qDebug("I've been unloaded!");
    return TRUE;
}

#if defined(__cplusplus)
}
#endif // __cplusplus


