#include "qwidgetplugin.h"
#include "qwidgetinterface_p.h"
#include "qobjectcleanuphandler.h"
#include "qwidget.h"

class QWidgetPluginPrivate : public QWidgetFactoryInterface, QLibraryInterface
{
public:
    QWidgetPluginPrivate( QWidgetPlugin *p )
	: plugin( p )
    {
    }

    virtual ~QWidgetPluginPrivate();

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    Q_REFCOUNT;

    QStringList featureList() const;
    QWidget *create( const QString &key, QWidget *parent, const char *name );
    QString group( const QString &widget ) const;
    QIconSet iconSet( const QString &widget ) const;
    QString includeFile( const QString &widget ) const;
    QString toolTip( const QString &widget ) const;
    QString whatsThis( const QString &widget ) const;
    bool isContainer( const QString &widget ) const;

    bool init();
    void cleanup();
    bool canUnload() const;

private:
    QWidgetPlugin *plugin;
    QObjectCleanupHandler widgets;
};

QRESULT QWidgetPluginPrivate::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( iid == IID_QUnknown )
	*iface = (QWidgetFactoryInterface*)this;
    else if ( iid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( iid == IID_QWidgetFactory )
	*iface = (QWidgetFactoryInterface*)this;
    else if ( iid == IID_QLibrary )
	*iface = (QLibraryInterface*)this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

QWidgetPluginPrivate::~QWidgetPluginPrivate()
{
    delete plugin;
}

QStringList QWidgetPluginPrivate::featureList() const
{
    return plugin->keys();
}

QWidget *QWidgetPluginPrivate::create( const QString &key, QWidget *parent, const char *name )
{
    QWidget *w = plugin->create( key, parent, name );
    widgets.add( w );
    return w;
}

QString QWidgetPluginPrivate::group( const QString &widget ) const
{
    return plugin->group( widget );
}

QIconSet QWidgetPluginPrivate::iconSet( const QString &widget ) const
{
    return plugin->iconSet( widget );
}

QString QWidgetPluginPrivate::includeFile( const QString &widget ) const
{
    return plugin->includeFile( widget );
}

QString QWidgetPluginPrivate::toolTip( const QString &widget ) const
{
    return plugin->toolTip( widget );
}

QString QWidgetPluginPrivate::whatsThis( const QString &widget ) const
{
    return plugin->whatsThis( widget );
}

bool QWidgetPluginPrivate::isContainer( const QString &widget ) const
{
    return plugin->isContainer( widget );
}

bool QWidgetPluginPrivate::init()
{
    return TRUE;
}

void QWidgetPluginPrivate::cleanup()
{
    widgets.clear();
}

bool QWidgetPluginPrivate::canUnload() const
{
    return widgets.isEmpty();
}


QWidgetPlugin::QWidgetPlugin()
{
    d = new QWidgetPluginPrivate( this );
}

QWidgetPlugin::~QWidgetPlugin()
{
    // don't delete d, as this is deleted by d
}

QStringList QWidgetPlugin::keys() const
{
    return QStringList();
}

QWidget *QWidgetPlugin::create( const QString &key, QWidget *parent, const char *name )
{
    return 0;
}

QString QWidgetPlugin::group( const QString &widget ) const
{
    return QString::null;
}

QIconSet QWidgetPlugin::iconSet( const QString &widget ) const
{
    return QString::null;
}

QString QWidgetPlugin::includeFile( const QString &widget ) const
{
    return QString::null;
}

QString QWidgetPlugin::toolTip( const QString &widget ) const
{
    return QString::null;
}

QString QWidgetPlugin::whatsThis( const QString &widget ) const
{
    return QString::null;
}

bool QWidgetPlugin::isContainer( const QString &widget ) const
{
    return FALSE;
}

QUnknownInterface *QWidgetPlugin::iface()
{
    QUnknownInterface *i;
    d->queryInterface( IID_QUnknown, &i );
    return i;
}
