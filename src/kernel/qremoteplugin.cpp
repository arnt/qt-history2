#include "qremoteplugin.h"

#ifndef QT_NO_REMOTE

#include "qremoteinterface_p.h"
#include "qobjectcleanuphandler.h"
#include "qremoteinterface_p.h"

/* \internal
  \class QRemotePlugin qremoteplugin.h
  \brief The QRemotePlugin class provides an abstract base for custom
  QRemoteInterface plugins.
  \ingroup plugins

  The remote plugin is a simple plugin interface that makes it easy to
  create custom remote controls that can be loaded dynamically into
  applications with a QRemoteFactory.

  Writing a rc plugin is achieved by subclassing this base class,
  reimplementing the pure virtual functions keys() and create(), and
  exporting the class with the Q_EXPORT_PLUGIN macro.  See the \link
  plugins-howto.html plugins documentation\endlink for an example.
*/

/* \internal
    \fn QStringList QRemotePlugin::keys() const

  Returns the list of remote control keys this plugin supports.

  These keys are usually the class names of the custom controls that are
  implemented in the plugin.

  \sa create()
*/

/* \internal
    \fn QRemoteInterface* QRemotePlugin::create( const QString& key )

  Creates and returns a QRemoteInterface object for the rc key \a key. The
  rc key is usually the class name of the required control.

  \sa keys()
*/

class QRemotePluginPrivate : public QRemoteFactoryInterface, public QLibraryInterface
{
public:
    QRemotePluginPrivate( QRemotePlugin *p )
	: plugin( p )
    {
    }

    virtual ~QRemotePluginPrivate();

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    Q_REFCOUNT;

    QStringList featureList() const;
    QRemoteInterface *create( const QString &key );

    bool init();
    void cleanup();
    bool canUnload() const;

private:
    QRemotePlugin *plugin;
    QObjectCleanupHandler controls;
};

QRESULT QRemotePluginPrivate::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( iid == IID_QUnknown )
	*iface = (QRemoteFactoryInterface*)this;
    else if ( iid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( iid == IID_QRemoteFactory )
	*iface = (QRemoteFactoryInterface*)this;
    else if ( iid == IID_QLibrary )
	*iface = (QLibraryInterface*) this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

QRemotePluginPrivate::~QRemotePluginPrivate()
{
    delete plugin;
}

QStringList QRemotePluginPrivate::featureList() const
{
    return plugin->keys();
}

QRemoteInterface *QRemotePluginPrivate::create( const QString &key )
{
    QRemoteInterface *c = plugin->create( key );
    // We use singletons here, so 'create()' may return a pointer that is already in
    // controls. So we remove it first, and then insert it again.
    // There's no harm done if the pointer wasn't already in controls.
    controls.remove( c );
    controls.add( c );
    return c;
}

bool QRemotePluginPrivate::init()
{
    return TRUE;
}

void QRemotePluginPrivate::cleanup()
{
    controls.clear();
}

bool QRemotePluginPrivate::canUnload() const
{
    return controls.isEmpty();
}


/*! \internal
  Constructs a remote plugin. This is invoked automatically by
  the Q_EXPORT_PLUGIN macro.
*/
QRemotePlugin::QRemotePlugin() : QGPlugin(0)
{
    d = new QRemotePluginPrivate( this );
    setIface( (QRemoteFactoryInterface*)d );
}

/*! \internal
  Destroys the remote plugin.

  You never have to call this explicitly. Qt destroys a plugin
  automatically when it is no longer used.

*/
QRemotePlugin::~QRemotePlugin()
{
    // don't delete d, as this is deleted by d
}

#endif //QT_NO_REMOTE
