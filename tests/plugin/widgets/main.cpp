#include "../tools/designer/plugins/designerinterface.h"

#include <qapplication.h>
#include <qcleanuphandler.h>

#include <qcanvas.h>

class ExtraWidgetsInterface : public WidgetInterface
{
public:
    ExtraWidgetsInterface( QUnknownInterface *parent, const char *name = 0 );
    ~ExtraWidgetsInterface();

    bool disconnectNotify();

    QStringList featureList() const;
    QWidget* create( const QString &classname, QWidget* parent = 0, const char* name = 0 );
    QString group( const QString& );
    QString iconSet( const QString& );
    QIconSet iconset( const QString& );
    QString includeFile( const QString& );
    QString toolTip( const QString& );
    QString whatsThis( const QString& );
    bool isContainer( const QString& );

    QGuardedCleanUpHandler<QObject> objects;
};

ExtraWidgetsInterface::ExtraWidgetsInterface( QUnknownInterface *parent, const char *name )
: WidgetInterface( parent, name )
{
}

ExtraWidgetsInterface::~ExtraWidgetsInterface()
{
}

bool ExtraWidgetsInterface::disconnectNotify()
{
    qDebug( "ExtraWidgetsInterface::disconnectNotify()" );
    if ( !objects.isClean() )
	return FALSE;
    return TRUE;
}

QStringList ExtraWidgetsInterface::featureList() const
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

QString ExtraWidgetsInterface::iconSet( const QString& )
{
    return "pushbutton.xpm";
}

QIconSet ExtraWidgetsInterface::iconset( const QString& )
{
    return QIconSet();
}

QString ExtraWidgetsInterface::includeFile( const QString& )
{
    return "qcanvas.h";
}

QString ExtraWidgetsInterface::toolTip( const QString& )
{
    return QString::null;
}

QString ExtraWidgetsInterface::whatsThis( const QString& )
{
    return QString::null;
}

bool ExtraWidgetsInterface::isContainer( const QString& )
{ 
    return FALSE;
}

class ExtraWidgetsPlugIn : public QPlugInInterface
{
public:
    ExtraWidgetsPlugIn();
    ~ExtraWidgetsPlugIn();
    QString name() const { return "Extra-Widgets plugin"; }
    QString description() const { return "QCanvas support for the Qt Designer"; }
    QString author() const { return "Trolltech"; }
};

ExtraWidgetsPlugIn::ExtraWidgetsPlugIn()
: QPlugInInterface( "ExtraWidgetsPlugIn" )
{
    new ExtraWidgetsInterface( this, "ExtraWidgetsInterface" );
}

ExtraWidgetsPlugIn::~ExtraWidgetsPlugIn()
{
}

Q_EXPORT_INTERFACE(ExtraWidgetsPlugIn)

