#include "../designerinterface.h"

#include <qcleanuphandler.h>
#include <qapplication.h>

#include "rc2ui.h"

class RCInterface : public FilterInterface
{
public:
    RCInterface();
    ~RCInterface();

    QStringList featureList() const;

    QStringList import( const QString& filter, const QString& filename );

private:
    QGuardedPtr<QApplicationInterface> appInterface;
};

RCInterface::RCInterface()
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

class RCPlugIn : public QPlugInInterface
{
public:
    RCPlugIn();
    ~RCPlugIn();

    QString name() const { return "MS Resource File import"; }
    QString description() const { return "Qt Designer import filter for Microsoft Resource Files"; }
    QString author() const { return "Trolltech"; }

    QUnknownInterface* queryInterface( const QString& );
    QStringList interfaceList() const;

private:
    RCInterface* rc;
};

RCPlugIn::RCPlugIn()
: rc( 0 )
{
}

RCPlugIn::~RCPlugIn()
{
    delete rc;
}

QStringList RCPlugIn::interfaceList() const
{
    QStringList list;

    list << "RCInterface";

    return list;
}

QUnknownInterface* RCPlugIn::queryInterface( const QString &request )
{
    if ( request == "RCInterface" )
	return rc ? rc : ( rc = new RCInterface );
    return 0;
}

Q_EXPORT_INTERFACE( RCPlugIn )
