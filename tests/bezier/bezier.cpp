#include <qapplication.h>
#include <qwidget.h>
#include <qpointarray.h>
#include <qpainter.h>


class BezierViewer : public QWidget {
public:
    BezierViewer( const QPointArray& a, QWidget* parent=0, const char* name=0 );
    void paintEvent( QPaintEvent* );
private:
    QPointArray points;
};


BezierViewer::BezierViewer( const QPointArray& a, QWidget* parent, const char* name )
	: QWidget( parent, name ), points( a )
{
    setBackgroundColor( Qt::white );
}


void BezierViewer::paintEvent( QPaintEvent* )
{
    if ( points.size() != 4 ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QPointArray::bezier: The array must have 4 control points" );
#endif
	return;
    }

    /* Calculate Bezier curve */
    QPointArray bezier = points.quadBezier();

    QPainter painter( this );

    /* Calculate scale to fit in window */
    QRect br = bezier.boundingRect() | points.boundingRect();
    QRect pr = rect();
    int scl = QMAX( QMIN(pr.width()/br.width(), pr.height()/br.height()), 1 );
    int border = scl-1;

    /* Scale Bezier curve vertices */
    for ( QPointArray::Iterator i = bezier.begin(); i != bezier.end(); ++i ) {
	i->setX( (i->x()-br.x()) * scl + border );
	i->setY( (i->y()-br.y()) * scl + border );
    }

    /* Draw grid */
    painter.setPen( Qt::lightGray );
    for ( int i = border; i <= pr.width(); i += scl ) {
	painter.drawLine( i, 0, i, pr.height() );
    }
    for ( int j = border; j <= pr.height(); j += scl ) {
	painter.drawLine( 0, j, pr.width(), j );
    }

    /* Write number of vertices */
    painter.setPen( Qt::red );
    painter.setFont( QFont("Helvetica", 14, QFont::DemiBold, TRUE ) );
    QString caption;
    caption.setNum( bezier.size() );
    caption += QString::fromLatin1( " vertices" );
    painter.drawText( 10, pr.height()-10, caption );

    /* Draw Bezier curve */
    painter.setPen( Qt::black );
    painter.drawPolyline( bezier );

    /* Scale and draw control points */
    painter.setPen( Qt::darkGreen );
    for ( QPointArray::Iterator p = points.begin(); p != points.end(); ++p ) {
	int x = (p->x()-br.x()) * scl + border;
	int y = (p->y()-br.y()) * scl + border;
	painter.drawLine( x-4, y-4, x+4, y+4 );
	painter.drawLine( x+4, y-4, x-4, y+4 );
    }

    /* Draw vertices */
    painter.setPen( Qt::red );
    painter.setBrush( Qt::red );
    for ( QPointArray::Iterator p = bezier.begin(); p != bezier.end(); ++p )
	painter.drawEllipse( p->x()-1, p->y()-1, 3, 3 );
}


#define EXAMPLE 3

int main( int argc, char *argv[] )
{
#if EXAMPLE > 2
    int x1 = 186; int y1 = 108;
    int x2 = 198; int y2 = 108;
    int x3 = 204; int y3 = 114;
    int x4 = 204; int y4 = 126;
#elif EXAMPLE > 1
    int x1 = 100; int y1 = 101;
    int x2 = 150; int y2 = 200;
    int x3 = 200; int y3 = 150;
    int x4 = 101; int y4 = 100;
#elif EXAMPLE > 0
    int x1 = 100; int y1 = 101;
    int x2 = 100; int y2 = 200;
    int x3 = 200; int y3 = 100;
    int x4 = 101; int y4 = 100;
#else
    int x1 = 100; int y1 = 101;
    int x2 = 101; int y2 = 101;
    int x3 = 101; int y3 = 100;
    int x4 = 100; int y4 = 100;
#endif

    QPointArray array( 4 );
    array.setPoint( 0, x1, y1 );
    array.setPoint( 1, x2, y2 );
    array.setPoint( 2, x3, y3 );
    array.setPoint( 3, x4, y4 );

    QApplication application( argc, argv );
    BezierViewer widget( array, 0 );

    application.setMainWidget( &widget );
    widget.show();
    return application.exec();
}
