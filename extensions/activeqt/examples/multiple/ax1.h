#include <qwidget.h>
#include <qpainter.h>

class QAxWidget1 : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("ClassID", "{1D9928BD-4453-4bdd-903D-E525ED17FDE5}")
    Q_CLASSINFO("InterfaceID", "{99F6860E-2C5A-42ec-87F2-43396F4BE389}")
    Q_CLASSINFO("EventsID", "{0A3E9F27-E4F1-45bb-9E47-63099BCCD0E3}")

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
