#include "helpwindow.h"
#include <qurl.h>
#include <qmessagebox.h>

HelpWindow::HelpWindow( QWidget *parent = 0, const char *name = 0 )
    : QTextBrowser( parent, name )
{
}

void HelpWindow::setSource( const QString &name )
{
    QUrl u( context(), name );
    if ( !u.isLocalFile() ) {
	QMessageBox::information( this, tr( "Help" ), tr( "Can't load and display non-local file\n"
							  "%1" ).arg( name ) );
	return;	
    }

    QTextBrowser::setSource( name );
}

