#include <qpainter.h>

#include "distributionwidget.h"

DistributionWidget::DistributionWidget( QWidget* parent, const char* name, WFlags f ) : QWidget( parent, name, f )
{
    backColor = QColor( 0x00, 0x00, 0x00 );
    usedColor = QColor( 0xff, 0x40, 0x00 );
    otherColor = QColor( 0x00, 0x40, 0xff );
    setTotalBlocks( 0 );
}

DistributionWidget::~DistributionWidget()
{
}

void DistributionWidget::setTotalBlocks( int blocks )
{
    totalBlocks = blocks;
    blockStatus.truncate( 0 );
    blockStatus.resize( blocks );
    for( int i = 0; i < totalBlocks; i++ )
	blockStatus[ i ] = BlockStatus_Free;
}

void DistributionWidget::setBlockStatus( int block, int status )
{
    if( block < totalBlocks )
	blockStatus[ block ] = status;
}

void DistributionWidget::paintEvent( QPaintEvent* pEv )
{
    QPainter p( this );
    QRect rc( rect() );
    int elements = rc.width() * rc.height();
    double stepSize = double( totalBlocks ) / double( elements );
    int xp, yp;

    p.fillRect( rect(), QBrush( backColor ) );

    if( stepSize ) {
	xp = rc.left();
	yp = rc.top();
	for( int e = 0; e < elements; e++ ) {
	    int status = 0;
	    for( int i = ( e * stepSize ); i < ( e + 1 ) * stepSize; i++ ) {
		if( blockStatus[ i ] > status )
		    status = blockStatus[ i ];
	    }
	    xp++;
	    if( xp == rc.width() ) {
		xp = rc.left();
		yp++;
	    }

	    switch( status ) {
	    case BlockStatus_Other:
		p.setPen( otherColor );
		p.drawPoint( xp, yp );
		break;
	    case BlockStatus_Used:
		p.setPen( usedColor );
		p.drawPoint( xp, yp );
		break;
	    }
	}
    }
}

QSize DistributionWidget::sizeHint() const
{
    return QSize( 200, 64 );
}