#include "qstyleplugin.h"
#include "qstyleinterface_p.h"
#include "qobjectcleanuphandler.h"
#include "qstyle.h"

class QStylePluginPrivate : public QStyleFactoryInterface, public QLibraryInterface
{
public:
    QStylePluginPrivate( QStylePlugin *p )
	: plugin( p )
    {
    }

    virtual ~QStylePluginPrivate();

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    Q_REFCOUNT;

    QStringList featureList() const;
    QStyle *create( const QString &key );

    bool init();
    void cleanup();
    bool canUnload() const;

private:
    QStylePlugin *plugin;
    QObjectCleanupHandler styles;
};

QRESULT QStylePluginPrivate::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( iid == IID_QUnknown )
	*iface = (QStyleFactoryInterface*)this;
    else if ( iid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( iid == IID_QStyleFactory )
	*iface = (QStyleFactoryInterface*)this;
    else if ( iid == IID_QLibrary )
	*iface = (QLibraryInterface*) this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

QStylePluginPrivate::~QStylePluginPrivate()
{
    delete plugin;
}

QStringList QStylePluginPrivate::featureList() const
{
    return plugin->keys();
}

QStyle *QStylePluginPrivate::create( const QString &key )
{
    QStyle *st = plugin->create( key );
    styles.add( st );
    return st;
}

bool QStylePluginPrivate::init()
{
    return TRUE;
}

void QStylePluginPrivate::cleanup()
{
    styles.clear();
}

bool QStylePluginPrivate::canUnload() const
{
    return styles.isEmpty();
}


QStylePlugin::QStylePlugin()
{
    d = new QStylePluginPrivate( this );
}

QStylePlugin::~QStylePlugin()
{
    // don't delete d, as this is deleted by d
}

QStringList QStylePlugin::keys() const
{
    return QStringList();
}

QStyle *QStylePlugin::create( const QString &key )
{
    return 0;
}

QUnknownInterface *QStylePlugin::iface()
{
    QUnknownInterface *i;
    d->queryInterface( IID_QUnknown, &i );
    return i;
}
