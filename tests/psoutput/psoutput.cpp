#include <qpainter.h>
#include <qprinter.h>
#include <qapplication.h>


void drawPoint( QPainter & p, const QRect &r )
{
    p.setPen( Qt::black );
    p.drawPoint( r.center() );
    p.setPen( QPen( Qt::black, 3 ) );
    p.drawPoint( r.topLeft() );
    p.setPen( QPen( Qt::black, 3 ) );
    p.drawPoint( r.bottomRight() );
    p.setPen( Qt::white );
    p.drawPoint( r.bottomRight() );
}


void drawPoints( QPainter & p, const QRect &r )
{
    QPointArray a( 3 );
    a[0] = r.topLeft();
    a[1] = r.center();
    a[2] = r.bottomRight();
    p.setPen( Qt::black );
    p.drawPoints( a );
}


void drawLine( QPainter & p, const QRect &r )
{
    p.setPen( Qt::black );
    p.drawLine( r.topLeft(), r.bottomRight() );
    p.setPen( Qt::red );
    p.drawLine( r.topLeft(), QPoint( r.right(), r.center().y() ) );
    p.setPen( Qt::green );
    p.drawLine( r.topLeft(), QPoint( r.center().x(), r.bottom() ) );
}


void drawRect( QPainter & p, const QRect &r )
{
    p.setPen( QPen( Qt::black, 3 ) );
    p.drawRect( r );
    p.setPen( Qt::white );
    p.drawRect( r );
}


void drawWinFocusRect( QPainter & p, const QRect &r )
{
    p.drawWinFocusRect( r );
}


void drawEllipse( QPainter & p, const QRect &r )
{
    p.setPen( QPen( Qt::black, 3 ) );
    p.drawEllipse( r.left(), r.top(), r.width()/2, r.height() );
    p.setPen( Qt::black );
    p.drawEllipse( r.right()-r.width()/2, r.top(), r.width()/2, r.height() );
    p.setPen( Qt::white );
    p.drawEllipse( r.left(), r.top(), r.width()/2, r.height() );
}


void drawArc( QPainter & p, const QRect &r )
{
    p.setPen( QPen( Qt::black, 3 ) );
    p.drawArc( r, 1600, 2560 );
    p.setPen( Qt::white );
    p.drawArc( r, 1600, 2560 );
}


void drawChord( QPainter & p, const QRect &r )
{
    p.setPen( QPen( Qt::black, 3 ) );
    p.drawChord( r, 1600, 2560 );
    p.setPen( Qt::white );
    p.drawChord( r, 1600, 2560 );
}


void drawLineSegments( QPainter & p, const QRect &r )
{
    QPointArray a( 6 );
    a[0] = r.topLeft();
    a[1] = r.bottomLeft();
    a[2] = QPoint( r.center().x(), r.top() );
    a[3] = QPoint( r.center().x(), r.bottom() );
    a[4] = r.topRight();
    a[5] = r.bottomRight();
    p.setPen( QPen( Qt::black, 3 ) );
    p.drawLineSegments( a );
    p.setPen( Qt::white );
    p.drawLineSegments( a );
}


void drawPolyline( QPainter & p, const QRect &r )
{
    QPointArray a( 6 );
    a[0] = r.topLeft();
    a[1] = r.bottomLeft();
    a[2] = QPoint( r.center().x(), r.top() );
    a[3] = QPoint( r.center().x(), r.bottom() );
    a[4] = r.topRight();
    a[5] = r.bottomRight();
    p.setPen( QPen( Qt::black, 3 ) );
    p.drawPolyline( a );
    p.setPen( Qt::white );
    p.drawPolyline( a );
}


void drawPolygon( QPainter & p, const QRect &r )
{
    double magic = .57735026918962576451;
    
    QPointArray a( 3 );
    a[0] = QPoint( r.left(), r.center().y() + (int)(r.height()/2*magic) );
    a[1] = QPoint( r.right(), r.center().y() + (int)(r.height()/2*magic) );
    a[2] = QPoint( r.center().x(), r.top() );
    QPointArray b( 3 );
    b[0] = QPoint( r.left(), r.center().y() - (int)(r.height()/2*magic) );
    b[1] = QPoint( r.right(), r.center().y() - (int)(r.height()/2*magic) );
    b[2] = QPoint( r.center().x(), r.bottom() );
    p.setPen( QPen( Qt::black, 3 ) );
    p.drawPolygon( a );
    p.drawPolygon( b );
    p.setPen( Qt::white );
    p.drawPolygon( a );
    p.drawPolygon( b );
}


void drawQuadBezier( QPainter & p, const QRect &r )
{
    QPointArray a( 4 );
    a[0] = r.topLeft();
    a[1] = r.bottomLeft();
    a[2] = r.bottomRight();
    a[3] = r.topRight();
    p.setPen( QPen( Qt::black, 3 ) );
    p.drawQuadBezier( a );
    p.setPen( Qt::white );
    p.drawQuadBezier( a );
    a[1] = r.bottomRight();
    a[2] = r.bottomLeft();
    p.setPen( QPen( Qt::blue, 3 ) );
    p.drawQuadBezier( a );
    p.setPen( Qt::white );
    p.drawQuadBezier( a );
}


void drawPixmap( QPainter & p, const QRect &r )
{
}


void drawImage( QPainter & p, const QRect &r )
{
}


void drawTiledPixmap( QPainter & p, const QRect &r )
{
}


void drawPicture( QPainter & p, const QRect &r )
{
}


void drawText( QPainter & p, const QRect &r )
{
}


typedef void (*TestFunction)(QPainter &, const QRect &);


TestFunction f[18] = {
    drawPoint, drawPoints, drawLine, drawRect, drawWinFocusRect,
    drawEllipse, drawArc, drawChord, drawLineSegments, drawPolyline,
    drawPolygon, drawQuadBezier, drawPixmap, drawImage,
    drawTiledPixmap, drawPicture, drawText
};


const char * n[16] = {
    "drawPoint", "drawPoints", "drawLine", "drawRect", "drawWinFocusRect",
    "drawEllipse", "drawArc", "drawChord", "drawLineSegments", "drawPolyline",
    "drawPolygon", "drawQuadBezier", "drawPixmap", "drawImage",
    "drawTiledPixmap", "drawPicture", "drawText"
};



void test( const char * output,
	   QPrinter::Orientation o, QPrinter::ColorMode cm,
	   int copies, bool multipage, const char * creator )
{
    QPrinter printer;

    if ( output ) {
	printer.setOutputFileName( output );
	printer.setOutputToFile( TRUE );
    }

    printer.setOrientation( o );
    printer.setColorMode( cm );
    printer.setNumCopies( copies );
    if ( creator ) {
	printer.setCreator( creator );
    }

    QPainter p( &printer );
    p.setFont( QFont( "Helvetica", 8 ) );

    p.setPen( Qt::lightGray );
    int i;
    QRect cr( 28, 28, 127, 127 );
    QRect pr( 14, 14, cr.width()-28, cr.width()-28 );
    for( i=0; i<16; i++ ) {
	p.setPen( Qt::lightGray );
	p.drawRect( cr );
	p.setPen( Qt::darkGray );
	p.drawText( cr.left()+5, cr.top()+12, n[i] );
	p.save();
	//p.setClipRect( cr );
	p.translate( cr.left(), cr.top() );
	f[i]( p, pr );
	p.restore();
	
	// A4, two 28-point margins
	if ( cr.left() + 2*cr.width() >= 539 )
	    cr.setRect( 28, cr.bottom()+1, cr.width(), cr.height() );
	else
	    cr.setRect( cr.right()+1, cr.top(), cr.width(), cr.height() );
    }
}

main(int argc, char** argv)
{
    QApplication app(argc, argv);

    test( "/tmp/portrait-1.ps", QPrinter::Portrait, QPrinter::GrayScale,
	  1, FALSE, 0 );
    test( "/tmp/landscape-1.ps", QPrinter::Landscape, QPrinter::GrayScale,
	  1, FALSE, 0 );

}
