#include <filterinterface.h>

#include <qapplication.h>

#include "rc2ui.h"

class RCFilter : public ImportFilterInterface, public QLibraryInterface
{
public:
    RCFilter();

    QRESULT queryInterface( const QUuid&, QUnknownInterface **iface );
    Q_REFCOUNT;

    QStringList featureList() const;
    QStringList import( const QString& filter, const QString& filename );

    bool init();
    void cleanup();
    bool canUnload() const;
};

RCFilter::RCFilter()
{
}

QRESULT RCFilter::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)(ImportFilterInterface*)this;
    else if ( uuid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_ImportFilter )
	*iface = (ImportFilterInterface*)this;
    else if ( uuid == IID_QLibrary )
	*iface = (QLibraryInterface*)this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

QStringList RCFilter::featureList() const
{
    QStringList list;
    list << "Microsoft Resource Files (*.rc)" ;
    return list;
}

QStringList RCFilter::import( const QString &, const QString& filename )
{
    QFile file( filename );
    if ( !file.open( IO_ReadOnly ) )
	qWarning( "uic: Could not open file '%s' ", filename.latin1() );
    QTextStream in;
    in.setDevice( &file );

    RC2UI c( &in );
    QStringList files;
    c.parse();
    return c.targetFiles;
}

bool RCFilter::init()
{
    return TRUE;
}

void RCFilter::cleanup()
{
}

bool RCFilter::canUnload() const
{
    return TRUE;
}

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( RCFilter )
}
