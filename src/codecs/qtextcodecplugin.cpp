#include "qtextcodecplugin.h"
#include "qtextcodecinterface_p.h"

/*!   \class QTextCodecPlugin qtextcodecplugin.h
  \brief The QTextCodecPlugin class provides an abstract base for custom QTextCodec plugins.
  \ingroup plugins
  \mainclass

  The style plugin is a simple plugin interface that makes it easy to
  create custom text codecs that can be loaded dynamically into
  applications.
  
  Writing a text codec plugin is achieved by subclassing this
  baseclass, reimplementing the pure virtual functions names(),
  createForName(), mibEnums() and createForMib() , and exporting the
  class with the Q_EXPORT_PLUGIN macro.  See the \link plugins.html Qt
  Plugins documentation \endlink for details.

  See the \link ftp://ftp.isi.edu/in-notes/iana/assignments/character-sets IANA
  character-sets encoding file\endlink for more information on mime
  names and mib enums.

*/

/*! \fn QStringList QTextCodecPlugin::names() const
  
  Returns the list of mime names this plugin supports. 
  
  
  \sa createForName()
*/

/*! \fn QTextCodec *QTextCodecPlugin::createForName( const QString &name );
  
  Creates a QTextCodec object for \a name.
  
  \sa names()
*/


/*! \fn QValueList<int> QTextCodecPlugin::mibEnums() const
  
  Returns the list of mib enums this plugin supports.
  
  
  \sa createForMib()
*/

/*! \fn QTextCodec *QTextCodecPlugin::createForMib( int mib );
  
  Creates a QTextCodec object for the mib enum \a mib.
  (see \link ftp://ftp.isi.edu/in-notes/iana/assignments/character-sets
  the IANA character-sets encoding file\endlink for more information)
  
  \sa mibEnums()
*/
  


class QTextCodecPluginPrivate : public QTextCodecFactoryInterface
{
public:
    QTextCodecPluginPrivate( QTextCodecPlugin *p )
	: plugin( p )
    {
    }
    virtual ~QTextCodecPluginPrivate();

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    Q_REFCOUNT;

    QStringList featureList() const;
    QTextCodec *createForMib( int mib );
    QTextCodec *createForName( const QString &name );

private:
    QTextCodecPlugin *plugin;
};

QTextCodecPluginPrivate::~QTextCodecPluginPrivate()
{
    delete plugin;
}

QRESULT QTextCodecPluginPrivate::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( iid == IID_QUnknown )
	*iface = this;
    else if ( iid == IID_QFeatureList )
	*iface = this;
    else if ( iid == IID_QTextCodecFactory )
	*iface = this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

QStringList QTextCodecPluginPrivate::featureList() const
{
    QStringList keys = plugin->names();
    QValueList<int> mibs = plugin->mibEnums();
    for ( QValueList<int>::Iterator it = mibs.begin(); it != mibs.end(); ++it )
	keys += QString("MIB-%1").arg( *it );
    return keys;
}

QTextCodec *QTextCodecPluginPrivate::createForMib( int mib )
{
    return plugin->createForMib( mib );
}

QTextCodec *QTextCodecPluginPrivate::createForName( const QString &name )
{
    return plugin->createForName( name );
}


/*!  
  Constructs a text codec plugin. This is invoked automatically by
  the Q_EXPORT_PLUGIN macro.
 */
QTextCodecPlugin::QTextCodecPlugin()
{
    d = new QTextCodecPluginPrivate( this );
    _iface = d;
}

/*!
  Destroys the text codec plugin.
  
  You never have to call this explicitely. Qt destroys a plugin
  automatically when it is no longer used.
  
*/
QTextCodecPlugin::~QTextCodecPlugin()
{
}

