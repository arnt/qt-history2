#include "../designerinterface.h"

#include <qapplication.h>
#include <qcleanuphandler.h>

#include <qcanvas.h>

class ExtraWidgetsInterface : public WidgetInterface
{
public:
    ExtraWidgetsInterface();
    ~ExtraWidgetsInterface();

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

ExtraWidgetsInterface::ExtraWidgetsInterface()
{
}

ExtraWidgetsInterface::~ExtraWidgetsInterface()
{
}

bool ExtraWidgetsInterface::connectNotify( QApplication* theApp )
{
    return TRUE;
}

bool ExtraWidgetsInterface::disconnectNotify( QApplication* theApp )
{
    if ( !objects.clean() )
	return FALSE;
    return TRUE;
}

QStringList ExtraWidgetsInterface::featureList()
{
    QStringList list;

    list << "QCanvasView";

    return list;
}

QWidget* ExtraWidgetsInterface::create( const QString &description, QWidget* parent, const char* name )
{
    QWidget* w = 0;
    if ( description == "QCanvasView" ) {
	QCanvas* canvas = new QCanvas;
	objects.addCleanUp( canvas );
	w = new QCanvasView( canvas, parent, name );
    }

    objects.addCleanUp( w );
    return w;
}

QString ExtraWidgetsInterface::group( const QString& description )
{
    if ( description == "QCanvasView" )
	return "Views";

    return QString::null;
}

QString ExtraWidgetsInterface::iconSet( const QString& description )
{
    return "pushbutton.xpm";
}

QString ExtraWidgetsInterface::includeFile( const QString& description )
{
    return QString::null;
}

QString ExtraWidgetsInterface::toolTip( const QString& description )
{
    if ( description == "QCanvasView" )
	return "Canvas";

    return QString::null;
}

QString ExtraWidgetsInterface::whatsThis( const QString& description )
{
    return QString::null;
}

bool ExtraWidgetsInterface::isContainer( const QString& description )
{ 
    return FALSE;
}

Q_EXPORT_INTERFACE(WidgetInterface, ExtraWidgetsInterface)

