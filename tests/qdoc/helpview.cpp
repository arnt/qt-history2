#include "helpview.h"
#include <qfile.h>
#include <qurl.h>
#include <qmessagebox.h>

HelpView::HelpView( QWidget *parent, const QString &dd )
    : QTextBrowser( parent )
{
    docDir = dd;
}

void HelpView::showLink( const QString &link , const QString& title )
{
    QString file( docDir + "/" + link );
    QUrl u( file );
    if ( !QFile::exists( u.path() ) ) {
	qDebug( "could not find %s", u.path().latin1() );
	return;
    }

    setCaption( title );
    blockSignals( TRUE );
    setSource( u.path() );
    blockSignals( FALSE );
    if ( !u.ref().isEmpty() )
	scrollToAnchor( u.ref() );
}

void HelpView::setSource( const QString &name )
{
    QUrl u( context(), name );
    if ( !u.isLocalFile() ) {
	QMessageBox::critical( this, tr( "Error" ), tr( "Can't load and display non-local file\n"
							"%1" ).arg( name ) );
	return;
    }

    emit newSource( name );
    QTextBrowser::setSource( name );
    if ( !caption().isEmpty() && !hist.contains( u.fileName() ) )
	hist[ u.fileName() ] = caption();
}
