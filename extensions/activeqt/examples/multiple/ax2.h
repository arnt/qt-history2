#include <qwidget.h>
#include <qpainter.h>

#include <qaxfactory.h>

class QAxWidget2 : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( int lineWidth READ lineWidth WRITE setLineWidth )
public:
    QAxWidget2( QWidget *parent = 0, const char *name = 0, WFlags f = 0 )
	: QWidget( parent, name, f ), line_width( 1 )
    {
    }

    int lineWidth() const
    {
	return line_width;
    }
    void setLineWidth( int lw )
    {
	line_width = lw;
	repaint();
    }

protected:
    void paintEvent( QPaintEvent *e )
    {
	QPainter paint( this );
	QPen pen = paint.pen();
	pen.setWidth( line_width );
	paint.setPen( pen );

	QRect r = rect();
	r.addCoords( 10, 10, -10, -10 );
	paint.drawEllipse( r );
    }

private:
    int line_width;
};
