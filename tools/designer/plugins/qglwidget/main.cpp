#include "../designerinterface.h"

#include <qcleanuphandler.h>

#include "glwidget.h"

class OpenGLWidgetInterface : public WidgetInterface
{
public:
    OpenGLWidgetInterface( QUnknownInterface *parent );
    ~OpenGLWidgetInterface();

    QStringList featureList() const;

    QWidget* create( const QString &classname, QWidget* parent = 0, const char* name = 0 );
    QString group( const QString& ) const;
    QString iconSet( const QString& ) const;
    QIconSet iconset( const QString& ) const;
    QString includeFile( const QString& ) const;
    QString toolTip( const QString& ) const;
    QString whatsThis( const QString& ) const;
    bool isContainer( const QString& ) const;

private:
    QGuardedCleanupHandler<QObject> objects;
};

OpenGLWidgetInterface::OpenGLWidgetInterface( QUnknownInterface *parent )
: WidgetInterface( parent )
{
}

OpenGLWidgetInterface::~OpenGLWidgetInterface()
{
}

QStringList OpenGLWidgetInterface::featureList() const
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

    objects.add( w );
    return w;
}

QString OpenGLWidgetInterface::group( const QString& description ) const
{
    if ( description == "QGLWidget" )
	return "Views";
    return QString::null;
}

QString OpenGLWidgetInterface::iconSet( const QString& description ) const
{
    if ( description == "QGLWidget" )
	return "pushbutton.xpm";
    return QString::null;
}

QIconSet OpenGLWidgetInterface::iconset( const QString& ) const
{
    return QIconSet();
}

QString OpenGLWidgetInterface::includeFile( const QString& description ) const
{
    if ( description == "QGLWidget" )
        return "qgl.h";
    return QString::null;
}

QString OpenGLWidgetInterface::toolTip( const QString& description ) const
{
    if ( description == "QGLWidget" )
	return QT_TR_NOOP("OpenGL Widget");
    return QString::null;
}

QString OpenGLWidgetInterface::whatsThis( const QString& description ) const
{
    if ( description == "QGLWidget" )
	return "A widget for OpenGL rendering";
    return QString::null;
}

bool OpenGLWidgetInterface::isContainer( const QString& ) const
{
    return FALSE;
}

class OpenGLPlugIn : public QUnknownInterface
{
public:
    OpenGLPlugIn();
    ~OpenGLPlugIn();
/*
    QString name() const { return "QGLWidget"; }
    QString description() const { return "Qt Designer plugin for the OpenGL widget"; }
    QString author() const { return "Trolltech"; }
*/
};

OpenGLPlugIn::OpenGLPlugIn()
: QUnknownInterface()
{
    new OpenGLWidgetInterface( this );
}

OpenGLPlugIn::~OpenGLPlugIn()
{
}

Q_EXPORT_INTERFACE( OpenGLPlugIn )
