#include <qwidget.h>
#include <qpainter.h>

#include <qaxfactory.h>

class QAxWidget1 : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QColor fillColor READ fillColor WRITE setFillColor )
public:
    QAxWidget1( QWidget *parent = 0, const char *name = 0, WFlags f = 0 )
	: QWidget( parent, name, f ), fill_color( red )
    {
    }

    QColor fillColor() const 
    { 
	return fill_color; 
    }
    void setFillColor( const QColor &fc ) 
    { 
	fill_color = fc; 
	repaint(); 
    }

protected:
    void paintEvent( QPaintEvent *e )
    {
	QPainter paint( this );
	QRect r = rect();
	r.addCoords( 10, 10, -10, -10 );
	paint.fillRect( r, fill_color );
    }

private:
    QColor fill_color;
};
