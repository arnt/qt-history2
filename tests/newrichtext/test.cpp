#include <qapplication.h>
#include <qpainter.h>

#include "qrtstring.h"

#if 1
class MyWidget : public QWidget
{
public:
    MyWidget( QWidget *parent = 0,  const char *name = 0);

    QRTString string;
protected:
    void paintEvent( QPaintEvent *e);

};


MyWidget::MyWidget( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
}


void MyWidget::paintEvent( QPaintEvent * )
{

    QPainter p( this );

    int x = 10, y = 50;
    // just for testing we draw char by char
    QRTFormat oldformat;
    for ( int i = 0; i < string.length(); i++ ) {
	QRTFormat format = string.format(i);
	if ( i == 0 || format != oldformat ) {
	    qDebug("format change at pos %d", i );
	}

	p.setFont( format.font() );
	p.setPen( format.color() );

	p.drawText( x, y, string.str(), i, 1 );
	x += p.fontMetrics().charWidth( string.str(), i );
	oldformat = format;
    }

}

#endif

int main( int argc, char **argv )
{
    {
	QApplication a(argc, argv);

	MyWidget *w = new MyWidget;
	w->resize( 300,  300 );
	w->show();
	a.setMainWidget ( w );

	QRTString string("This is a test string");

	QFont f;
	f.setPointSize( 24 );
	QRTFormat fmt( f, Qt::red );
	string.setFormat( fmt, 5, 5 );
	QRTFormat fmt2( f,  Qt::blue );
	QRTString str2("foo", fmt2 );
	string.insert( 20, str2 );

	w->string = string;
	a.exec();
	delete w;

    }
    qDebug("at exit:");
    QRTFormat::statistics();
}
