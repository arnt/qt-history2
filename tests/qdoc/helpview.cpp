#include "helpview.h"
#include <qfile.h>
#include <qurl.h>

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
    setSource( u.path() );
    if ( !u.ref().isEmpty() )
	scrollToAnchor( u.ref() );
}
