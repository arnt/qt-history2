#include "../designerinterface.h"

#include <qcleanuphandler.h>
#include <qapplication.h>

#include "rc2ui.h"

class RCInterface : public FilterInterface
{
public:
    RCInterface( QUnknownInterface *parent, const char *name = 0 );
    ~RCInterface();

    QStringList featureList() const;

    QStringList import( const QString& filter, const QString& filename );

private:
    QGuardedPtr<QApplicationInterface> appInterface;
};

RCInterface::RCInterface( QUnknownInterface *parent, const char *name )
: FilterInterface( parent, name )
{
}

RCInterface::~RCInterface()
{
}

QStringList RCInterface::featureList() const
{
    QStringList list;
    list << "Microsoft Resource Files (*.rc)" ;
    return list;
}

QStringList RCInterface::import( const QString &, const QString& filename )
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

class RCPlugIn : public QComponentInterface
{
public:
    RCPlugIn();
    ~RCPlugIn();

    QString name() const { return "MS Resource File import"; }
    QString description() const { return "Qt Designer import filter for Microsoft Resource Files"; }
    QString author() const { return "Trolltech"; }
};

RCPlugIn::RCPlugIn()
: QComponentInterface( "RCPlugIn" )
{
    new RCInterface( this, "RC Interface" );
}

RCPlugIn::~RCPlugIn()
{
}

Q_EXPORT_INTERFACE( RCPlugIn )
