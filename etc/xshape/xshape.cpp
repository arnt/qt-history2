//
// Qt Example Application: xshape
//
// Demonstrates the X shape extension.
//
// The ShapeWin widget handles resize pretty well. Moving the widget
// causes slow update of the background because the shape extension
// is very inefficient.
//
// NOTE: This program only works with X-Windows.
//

#include <qapplication.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qbitmap.h>
#define	 GC GC_QQQ				// avoid type mismatch
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/extensions/shape.h>


//
// ShapeWin - Window with an elliptic shape
//

class ShapeWin : public QWidget
{
public:
    ShapeWin();
protected:
    void  paintEvent( QPaintEvent * );
    void  resizeEvent( QResizeEvent * );
private:
    QBitmap mask;
    QPixmap pm;
};


//
// Paint/refresh widget.  It draws a color circle.
//

ShapeWin::ShapeWin()
{
    setBackgroundColor( white );

  // First we create the mask.

    setFont( QFont("Charter", 96, QFont::Bold) );

    const char *s = qApp->argc() == 2 ? qApp->argv()[1] : "Qt";
    QRect r = fontMetrics().boundingRect( s );

    mask.resize( r.width(), r.height() );	// bitmap fits text
    mask.fill( color0 );
    QPainter p;
    p.begin( &mask );
    p.setFont( font() );
    p.drawText( -r.x(), -r.y(), s );
    p.end();

    resize( 100,100 );
}

//
// Paint widget
//

void ShapeWin::paintEvent( QPaintEvent * )
{
    bitBlt( this, 0, 0, &pm );
}


//
// Resize widget
//

void ShapeWin::resizeEvent( QResizeEvent * )
{
    // Generate the pixmap we want to show

    pm.resize( width(), height() );

    QPainter p;
    QWMatrix m;
    int	     xc=width()/2, yc=height()/2;	// center point (xc,yc)
    const int angle_incr = 2;
    QPointArray a( 3 );

    m.rotate( -angle_incr );

    int x = QMAX(width(),height()) + 1;
    int y = 0;
    a[0] = QPoint(0,0);
    a[1] = QPoint(x,y);
    a[2] = m.map( a[1] );

    m.reset();
    m.translate( xc, yc );

    p.begin( &pm );
    p.setPen( NoPen );
    for ( int angle=0; angle<=360; angle += angle_incr ) {
	m.rotate( -angle_incr );
	p.setWorldMatrix( m );
	QColor c( angle, 255, 255, QColor::Hsv );
	p.setBrush( c );
	p.drawPolygon( a );
    }
    p.end();

    // Scale the mask bitmap

    m.reset();
    m.scale( (double)width()/mask.width(), (double)height()/mask.height() );
    QBitmap b;
    b = mask.xForm( m );

    // We have now scaled the bitmap to the same size as the widget.
    // Finally we set the X shape.

    XShapeCombineMask( x11Display(), winId(), ShapeBounding, 0, 0, b.handle(),
		       ShapeSet );
}


int main( int argc, char **argv )
{
    QApplication a( argc, argv );		// create application object

    ShapeWin shape;				// create and init a shapewin

    int event_base, error_base;			// shape extension supported?
    if ( !XShapeQueryExtension(shape.x11Display(),&event_base,&error_base) ) {
	warning( "shapewin: Cannot create shape window" );
	return 1;
    }

    a.setMainWidget( &shape );
    shape.show();				// show window
    return a.exec();				// go
}
