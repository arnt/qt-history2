#include "../designerinterface.h"

#include <qcleanuphandler.h>
#include <qapplication.h>

#include "rc2ui.h"

class RCInterface : public QObject, public FilterInterface
{
public:
    RCInterface();
    ~RCInterface();

    QString name() { return "RC converter"; }
    QString description() { return "Import/Export filter for Microsoft Resource Files"; }
    QString author() { return "Trolltech"; }

    bool connectNotify( QApplication* a );

    QStringList featureList();

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

bool RCInterface::connectNotify( QApplication* theApp )
{
    if ( !theApp )
	return FALSE;
    return TRUE;
}

QStringList RCInterface::featureList()
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

Q_EXPORT_INTERFACE(FilterInterface, RCInterface)
