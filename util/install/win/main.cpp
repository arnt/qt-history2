#include <qapplication.h>
#include <qmessagebox.h>
#include "setupwizardimpl.h"
#include "resource.h"
#include "globalinformation.h"
#include "environment.h"

#if defined Q_OS_WIN32
#include "archive.h"
#endif

GlobalInformation globalInformation;
SetupWizardImpl *wizard = 0;

int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    int res( -1 );

    for( int i = 0; i < app.argc(); i++ ) {
	if( QString( app.argv()[i] ) == "-reconfig" ) {
	    globalInformation.setReconfig( TRUE );

	    QString qmakespec = QEnvironment::getEnv( "QMAKESPEC" );
	    for (int mks = 0; mks <= GlobalInformation::MACX; ++mks) {
		if (globalInformation.text(GlobalInformation::Mkspec) == qmakespec) {
		    globalInformation.setSysId((GlobalInformation::SysId)mks);
		    break;
		}
	    }

	    if ( ++i < app.argc() ) {
		globalInformation.setQtVersionStr( app.argv()[i] );
	    }
	    break;
#if defined(Q_OS_WIN32)
	} else if ( QString( app.argv()[i] ) == "-add-archive" ) {
	    // -add-archive is an internal option to add the
	    // binary resource QT_ARQ
	    if ( ++i < app.argc() ) {
		if ( addArchive( app.argv()[i] ) )
		    return 0;
	    }
	    return res;
#endif
	}
    }

    wizard = new SetupWizardImpl( 0, 0, false, Qt::WStyle_NormalBorder | Qt::WStyle_Customize | Qt::WStyle_MinMax | Qt::WStyle_SysMenu | Qt::WStyle_Title );
    wizard->show();

    app.setMainWidget( wizard );
    res = app.exec();

    wizard->stopProcesses();

    //### memory leak

    return res;
}
