#define Q_INITGUID
#include "../designerinterface.h"
#undef Q_INITGUID

#include <qapplication.h>

#include "rc2ui.h"

class RCFilter : public ImportFilterInterface
{
public:
    RCFilter();

    QUnknownInterface *queryInterface( const QGuid& );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;

    QStringList import( const QString& filter, const QString& filename );

private:
    unsigned long ref;
};

RCFilter::RCFilter()
: ref( 0 )
{
}

QUnknownInterface *RCFilter::queryInterface( const QGuid &guid )
{
    QUnknownInterface *iface = 0;

    if ( guid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)this;
    if ( guid == IID_ImportFilterInterface )
	iface = (ImportFilterInterface*)this;

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
    if ( !--ref )
	delete this;

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

Q_EXPORT_INTERFACE( RCFilter )
