#include "../designerinterface.h"

#include <qapplication.h>
#include <qcleanuphandler.h>

#include "glwidget.h"

class OpenGLWidgetInterface : public WidgetInterface
{
public:
    OpenGLWidgetInterface();
    ~OpenGLWidgetInterface();

    bool connectNotify( QApplication* theApp );
    bool disconnectNotify( QApplication* theApp );

    QString name() { return "QGLWidget"; }
    QString description() { return "Qt Designer plugin for the OpenGL widget"; }
    QString author() { return "Trolltech"; }

    QStringList featureList();
    QWidget* create( const QString &classname, QWidget* parent = 0, const char* name = 0 );
    QString group( const QString& );
    QString iconSet( const QString& );
    QString includeFile( const QString& );
    QString toolTip( const QString& );
    QString whatsThis( const QString& );
    bool isContainer( const QString& );

private:
    QGuardedCleanUpHandler<QObject> objects;
};

OpenGLWidgetInterface::OpenGLWidgetInterface()
{
}

OpenGLWidgetInterface::~OpenGLWidgetInterface()
{
}

bool OpenGLWidgetInterface::connectNotify( QApplication* theApp )
{
    return TRUE;
}

bool OpenGLWidgetInterface::disconnectNotify( QApplication* theApp )
{
    if ( !objects.clean() )
	return FALSE;
    return TRUE;
}

QStringList OpenGLWidgetInterface::featureList()
{
    QStringList list;

    list << "QGLWidget";

    return list;
}

QWidget* OpenGLWidgetInterface::create( const QString &description, QWidget* parent, const char* name )
{
    QWidget* w = 0;

    if ( description == "QGLWidget" )
	w = new GLWidget( parent, name );

    objects.addCleanUp( w );
    return w;
}

QString OpenGLWidgetInterface::group( const QString& description )
{
    return QString::null;
}

QString OpenGLWidgetInterface::iconSet( const QString& )
{
    return QString::null;
}

QString OpenGLWidgetInterface::includeFile( const QString& description )
{
    return "qgl.h";
}

QString OpenGLWidgetInterface::toolTip( const QString& description )
{
    return QT_TR_NOOP("A widget for OpenGL rendering");
}

QString OpenGLWidgetInterface::whatsThis( const QString& )
{
    return QString::null;
}

bool OpenGLWidgetInterface::isContainer( const QString& description)
{
    return FALSE;
}

Q_EXPORT_INTERFACE(WidgetInterface, OpenGLWidgetInterface )
