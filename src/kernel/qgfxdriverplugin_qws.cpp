#include "qgfxdriverplugin_qws.h"
#include "qgfxdriverinterface_p.h"
#include "qgfx_qws.h"

/*!   \class QGfxDriverPlugin qimageformatplugin.h
  \brief The QGfxDriverPlugin class provides an abstract base for
  graphics driver plugins.
  \ingroup plugins

  The graphics driver plugin is a simple plugin interface that makes it easy to
  create custom graphics drivers.

  Writing a graphics driver plugin is achieved by subclassing this
  base class, reimplementing the pure virtual functions keys() and
  create(), and exporting the class with the Q_EXPORT_PLUGIN
  macro.  See the \link plugins-howto.html Plugins
  documentation\endlink for details.
*/

/*! \fn QStringList QGfxDriverPlugin::keys() const

  Returns the list of image formats this plugin supports.

  \sa create()
*/


/*! \fn  QScreen* QGfxDriverPlugin::create( const QString &driver )

  Creates a driver matching the type specified by \a driver.

  \sa keys()
*/

class QGfxDriverPluginPrivate : public QGfxDriverInterface
{
public:
    QGfxDriverPluginPrivate( QGfxDriverPlugin *p )
	: plugin( p )
    {
    }
    virtual ~QGfxDriverPluginPrivate();

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    Q_REFCOUNT;

#ifndef QT_NO_STRINGLIST
    QStringList featureList() const;
#endif

    QScreen* create( const QString& driver, int displayId );

private:
    QGfxDriverPlugin *plugin;
};

QGfxDriverPluginPrivate::~QGfxDriverPluginPrivate()
{
    delete plugin;
}

QRESULT QGfxDriverPluginPrivate::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( iid == IID_QUnknown )
	*iface = this;
    else if ( iid == IID_QFeatureList )
	*iface = this;
    else if ( iid == IID_QGfxDriver )
	*iface = this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

#ifndef QT_NO_STRINGLIST
QStringList QGfxDriverPluginPrivate::featureList() const
{
    return plugin->keys();
}
#endif

QScreen* QGfxDriverPluginPrivate::create( const QString& driver, int displayId )
{
    qDebug( "Loading plugin: %s", driver.latin1() );
    return plugin->create( driver, displayId );
}

/*!
  Constructs an image format plugin. This is invoked automatically by
  the Q_EXPORT_PLUGIN macro.
*/
QGfxDriverPlugin::QGfxDriverPlugin()
    : QGPlugin( d = new QGfxDriverPluginPrivate( this ) )
{
}

/*!
  Destroys the image format plugin.

  You never have to call this explicitly. Qt destroys a plugin
  automatically when it is no longer used.

*/
QGfxDriverPlugin::~QGfxDriverPlugin()
{
}


/*!\internal
 */
QScreen* QGfxDriverPlugin::create( const QString& driver, int displayId )
{
    Q_UNUSED( driver )
    Q_UNUSED( displayId )
    return 0;
}

