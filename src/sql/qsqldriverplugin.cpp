#include "qsqldriverplugin.h"
#include "qsqldriverinterface_p.h"

/*!   \class QSqlDriverPlugin qsqldriverplugin.h
  \brief The QSqlDriverPlugin class provides an abstract base for custom QSqlDriver plugins.
  \ingroup plugins
  \mainclass

  The SQL driver plugin is a simple plugin interface that makes it easy
  to create your own SQL driver plugins that can be loaded dynamically
  by Qt.
  
  Writing a SQL plugin is achieved by subclassing this baseclass, 
  reimplementing the pure virtual functions keys() and create(), and
  exporting the class with the Q_EXPORT_PLUGIN macro.  See the \link
  plugins-howto.html Plugins \endlink documentation for details.  
*/

/*! \fn QStringList QSqlDriverPlugin::keys() const

  Returns the  list of style keys this plugin supports.

  \sa create()
*/

/*! \fn QSqlDriver* QSqlDriverPlugin::create( const QString& key )

  Creates a QSqlDriver object for the driver key \a key.

  \sa keys()
*/

class QSqlDriverPluginPrivate : public QSqlDriverFactoryInterface
{
public:
    QSqlDriverPluginPrivate( QSqlDriverPlugin *p )
	: plugin( p )
    {
    }
    virtual ~QSqlDriverPluginPrivate();

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    Q_REFCOUNT;

    QStringList featureList() const;
    QSqlDriver *create( const QString &key );

private:
    QSqlDriverPlugin *plugin;
};

QSqlDriverPluginPrivate::~QSqlDriverPluginPrivate()
{
    delete plugin;
}

QRESULT QSqlDriverPluginPrivate::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( iid == IID_QUnknown )
	*iface = this;
    else if ( iid == IID_QFeatureList )
	*iface = this;
    else if ( iid == IID_QSqlDriverFactory )
	*iface = this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

QStringList QSqlDriverPluginPrivate::featureList() const
{
    return plugin->keys();
}

QSqlDriver *QSqlDriverPluginPrivate::create( const QString &key )
{
    return plugin->create( key );
}

/*!
  Constructs a SQL driver plugin. This is invoked automatically by
  the Q_EXPORT_PLUGIN macro.
*/

QSqlDriverPlugin::QSqlDriverPlugin()
{
    d = new QSqlDriverPluginPrivate( this );
    _iface = d;
}

/*!
  Destroys the SQL driver plugin.

  You never have to call this explicitly. Qt destroys a plugin
  automatically when it is no longer used.
*/
QSqlDriverPlugin::~QSqlDriverPlugin()
{
    // don't delete d, as this is deleted by d
}

