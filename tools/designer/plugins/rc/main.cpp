#include <filterinterface.h>

#include <qapplication.h>

#include "rc2ui.h"

class RCFilter : public ImportFilterInterface, public QLibraryInterface
{
public:
    RCFilter();

    QUnknownInterface *queryInterface( const QUuid& );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    QStringList import( const QString& filter, const QString& filename );

    bool init();
    void cleanup();
    bool canUnload() const;

private:
    unsigned long ref;
};

RCFilter::RCFilter()
: ref( 0 )
{
}

QUnknownInterface *RCFilter::queryInterface( const QUuid &uuid )
{
    QUnknownInterface *iface = 0;

    if ( uuid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)(ImportFilterInterface*)this;
    else if ( uuid == IID_QFeatureListInterface )
	iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_ImportFilterInterface )
	iface = (ImportFilterInterface*)this;
    else if ( uuid == IID_QLibraryInterface )
	iface = (QLibraryInterface*)this;

    if ( iface )
	iface->addRef();

    return iface;
}

unsigned long RCFilter::addRef()
{
    return ref++;
}

unsigned long RCFilter::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
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

Q_EXPORT_INTERFACE()
{
    QUnknownInterface *iface = (QUnknownInterface*)(ImportFilterInterface*)new RCFilter;
    iface->addRef();
    return iface;
}
