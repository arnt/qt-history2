#include <widgetinterface.h>

#include <qcleanuphandler.h>

#include "glwidget.h"

class OpenGLWidgetInterface : public WidgetInterface
{
public:
    OpenGLWidgetInterface();

    QUnknownInterface *queryInterface( const QUuid& );
    unsigned long addRef();
    unsigned long release();

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

    unsigned long ref;
};

OpenGLWidgetInterface::OpenGLWidgetInterface()
: ref( 0 )
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

QUnknownInterface *OpenGLWidgetInterface::queryInterface( const QUuid& uuid )
{
    QUnknownInterface *iface = 0;

    if ( uuid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)this;
    else if ( uuid == IID_QFeatureListInterface )
	iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_WidgetInterface )
	iface = (WidgetInterface*)this;

    if ( iface )
	iface->addRef();

    return iface;
}

unsigned long OpenGLWidgetInterface::addRef()
{
    return ref++;
}

unsigned long OpenGLWidgetInterface::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( OpenGLWidgetInterface );
}
