#include <widgetinterface.h>

#include <qobjectcleanuphandler.h>

#include "glwidget.h"

class OpenGLWidgetInterface : public WidgetInterface
{
public:
    OpenGLWidgetInterface();

    QRESULT queryInterface( const QUuid&, QUnknownInterface ** );
    Q_REFCOUNT;

    QStringList featureList() const;

    QWidget* create( const QString &classname, QWidget* parent = 0, const char* name = 0 );
    QString group( const QString& ) const;
    QIconSet iconSet( const QString& ) const;
    QString includeFile( const QString& ) const;
    QString toolTip( const QString& ) const;
    QString whatsThis( const QString& ) const;
    bool isContainer( const QString& ) const;

private:
    QObjectCleanupHandler objects;
};

OpenGLWidgetInterface::OpenGLWidgetInterface()
{
}

QStringList OpenGLWidgetInterface::featureList() const
{
    QStringList list;

    list << "GLWidget";

    return list;
}

QWidget* OpenGLWidgetInterface::create( const QString &description, QWidget* parent, const char* name )
{
    QWidget* w = 0;

    if ( description == "GLWidget" )
	w = new GLWidget( parent, name );

    objects.add( w );
    return w;
}

QString OpenGLWidgetInterface::group( const QString& description ) const
{
    if ( description == "GLWidget" )
	return "Views";
    return QString::null;
}

QIconSet OpenGLWidgetInterface::iconSet( const QString& ) const
{
    return QIconSet();
}

QString OpenGLWidgetInterface::includeFile( const QString& description ) const
{
    if ( description == "GLWidget" )
        return "qgl.h";
    return QString::null;
}

QString OpenGLWidgetInterface::toolTip( const QString& description ) const
{
    if ( description == "GLWidget" )
	return QT_TR_NOOP("OpenGL Widget");
    return QString::null;
}

QString OpenGLWidgetInterface::whatsThis( const QString& description ) const
{
    if ( description == "GLWidget" )
	return "A widget for OpenGL rendering";
    return QString::null;
}

bool OpenGLWidgetInterface::isContainer( const QString& ) const
{
    return FALSE;
}

QRESULT OpenGLWidgetInterface::queryInterface( const QUuid& uuid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)this;
    else if ( uuid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_Widget )
	*iface = (WidgetInterface*)this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( OpenGLWidgetInterface );
}
