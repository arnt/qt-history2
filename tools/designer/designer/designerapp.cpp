#include "designerapp.h"
#include "designerappiface.h"

#if defined(HAVE_KDE)
DesignerApplication::DesignerApplication( int &argc, char **argv, const QCString &rAppName )
    : KApplication( argc, argv, rAppName ), appIface( 0 )
{
}

#else

DesignerApplication::DesignerApplication( int &argc, char **argv )
    : QApplication( argc, argv ), appIface( 0 )
{
#if defined(_WS_WIN_)
    DESIGNER_OPENFILE = RegisterWindowMessage(LPCTSTR("QT_DESIGNER_OPEN_FILE"));
#endif
}

#endif


QApplicationInterface * DesignerApplication::requestApplicationInterface()
{
    return appIface ? appIface : ( appIface = new DesignerApplicationInterface );
}

#if defined(_WS_WIN_)
#include <qt_windows.h>
#include <process.h>
bool DesignerApplication::winEventFilter( MSG *msg )
{
    if ( msg->message == DESIGNER_OPENFILE ) {
	QFile f( QDir::homeDirPath() + "/.designerargs" );
	QFileInfo fi(f);
	if ( fi.lastModified() == lastMod )
	    return QApplication::winEventFilter( msg );
	lastMod = fi.lastModified();
	f.open( IO_ReadOnly );
	QString args;
	f.readLine( args, f.size() );
	QStringList lst = QStringList::split( " ", args );

	for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
	    QString arg = (*it).stripWhiteSpace();
	    if ( arg[0] != '-' ) {
		QObjectList* l = MainWindow::self->queryList( "FormWindow" );
		FormWindow* fw = (FormWindow*) l->first();
		FormWindow* totop;
		bool haveit = FALSE;
		while ( fw ) {
		    haveit = haveit || fw->fileName() == arg;
		    if ( haveit )
			totop = fw;

		    fw = (FormWindow*) l->next();
		}
		
		if ( !haveit ) {
		    FlashWindow( MainWindow::self->winId(), TRUE );
		    MainWindow::self->openFile( arg );
		} else {
		    totop->setFocus();
		}
		delete l;
	    }
	}
	return TRUE;
    }
    return QApplication::winEventFilter( msg );
}
#endif
