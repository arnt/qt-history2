#include "../../../../../tools/designer/designer/widgetinterface.h"

#include <qapplication.h>
#include <qcleanuphandler.h>

#include <qcanvas.h>
#include <qgl.h>
#include <qworkspace.h>
#include <qiconview.h>
#include <qtable.h>

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

    QString name() { return "Extra Widgets"; }
    QString description() { return "Qt Designer plugin for extra widgets"; }
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

    list << "QCanvasView";
    list << "QGLWidget";
    list << "QWorkspace";
    list << "QIconView";
    list << "QTable";

    return list;
}

QWidget* QWidgetsInterface::create( const QString &description, QWidget* parent, const char* name )
{
    QWidget* w = 0;
    if ( description == "QCanvasView" ) {
	QCanvas* canvas = new QCanvas;
	objects.addCleanUp( canvas );
	w = new QCanvasView( canvas, parent, name );
    } else if ( description == "QGLWidget" ) {
	w = new QGLWidget( parent, name );
    } else if ( description == "QWorkspace" ) {
	w = new QWorkspace( parent, name );
    } else if ( description == "QIconView" ) {
	w = new QIconView( parent, name );
    } else if ( description == "QTable" ) {
	w = new QTable( parent, name );
    } else {
	qWarning("Widget class %s not supported by this plugin!", description.latin1() );
    }

    objects.addCleanUp( w );
    return w;
}

QString QWidgetsInterface::group( const QString& description )
{
    if ( description == "QCanvasView" )
	return "Views";
    else if ( description == "QGLWidget" )
	return "Extended";
    else if ( description == "QWorkspace" )
	return "Containers";
    else if ( description == "QIconView" )
	return "Views";
    else if ( description == "QTable" )
	return "Views";

    return QString::null;
}

QString QWidgetsInterface::iconSet( const QString& description )
{
    return QString::null;
}

QString QWidgetsInterface::includeFile( const QString& description )
{
    return QString::null;
}

QString QWidgetsInterface::toolTip( const QString& description )
{
    if ( description == "QCanvasView" )
	return "Canvas";
    else if ( description == "QGLWidget" )
	return "OpenGL Widget";
    else if ( description == "QWorkspace" )
	return "Workspace";
    else if ( description == "QIconView" )
	return "Icon View";
    else if ( description == "QTable" )
	return "Table";

    return QString::null;
}

QString QWidgetsInterface::whatsThis( const QString& description )
{
    return QString::null;
}

bool QWidgetsInterface::isContainer( const QString& description )
{ 
    if ( description == "QWorkspace" )
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
