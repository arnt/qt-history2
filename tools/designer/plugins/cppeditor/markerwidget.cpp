#include "markerwidget.h"
#include "viewmanager.h"
#include <qrichtext_p.h>
#include "editor.h"
#include <qpainter.h>
#include "paragdata.h"

static const char * error_xpm[] = {
    "14 14 4 1",
    "       c None",
    ".      c #FFFFFF",
    "+      c #8B0000",
    "@      c #FF0000",
    "              ",
    "     ....     ",
    "    .++++.    ",
    "   .+@@@@+.   ",
    "  .+@@@@@@+.  ",
    " .+@@@@@@@@+. ",
    " .+@@@@@@@@+. ",
    " .+@@@@@@@@+. ",
    " .+@@@@@@@@+. ",
    "  .+@@@@@@+.  ",
    "   .+@@@@+.   ",
    "    .++++.    ",
    "     ....     ",
    "              "};

static QPixmap *errorPixmap = 0;

MarkerWidget::MarkerWidget( ViewManager *parent )
    : QWidget( parent ), viewManager( parent )
{
    if ( !errorPixmap ) {
	errorPixmap = new QPixmap( error_xpm );
    }
}

void MarkerWidget::paintEvent( QPaintEvent * )
{
    buffer.fill( backgroundColor() );

    QTextParag *p = ( (Editor*)viewManager->currentView() )->document()->firstParag();
    QPainter painter( &buffer );
    int yOffset = ( (Editor*)viewManager->currentView() )->contentsY();
    while ( p ) {
	ParagData *paragData = (ParagData*)p->extraData();
	if ( paragData ) {
	    switch ( paragData->marker ) {
	    case ParagData::Error:
		painter.drawPixmap( ( width() - errorPixmap->width() ) / 2, p->rect().y() - yOffset, *errorPixmap );
		break;
	    default:
		break;
	    }
	}
	p = p->next();
    }
    painter.end();

    bitBlt( this, 0, 0, &buffer );
}

void MarkerWidget::resizeEvent( QResizeEvent *e )
{
    buffer.resize( e->size() );
    QWidget::resizeEvent( e );
}
