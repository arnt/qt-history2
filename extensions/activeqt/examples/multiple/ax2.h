#include <qwidget.h>
#include <qpainter.h>

class QAxWidget2 : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("ClassID", "{58139D56-6BE9-4b17-937D-1B1EDEDD5B71}")
    Q_CLASSINFO("InterfaceID", "{B66280AB-08CC-4dcc-924F-58E6D7975B7D}")
    Q_CLASSINFO("EventsID", "{D72BACBA-03C4-4480-B4BB-DE4FE3AA14A0}")
    Q_CLASSINFO("ToSuperClass", "QAxWidget2")
    Q_CLASSINFO("StockEvents", "yes")

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

signals:
    void clicked();

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

    void mouseReleaseEvent(QMouseEvent *e)
    {
	emit clicked();
    }

private:
    int line_width;
};
