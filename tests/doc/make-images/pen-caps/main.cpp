/*
    Use this to create: pen-cap-styles.png
*/
#include <qapplication.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qframe.h>

class Frame : public QFrame
{
public:
    Frame( QWidget *parent, const char * name = 0, WFlags f = 0 ) :
	QFrame( parent, name, f ) {}

    void paintEvent( QPaintEvent * )
    {
	QPainter painter( this );
	painter.setFont( QFont( "Helvetica", 10, QFont::Bold ) );

	painter.setPen( Qt::darkBlue );
	QPen pen = painter.pen();

	painter.drawText(  10,  10, 150,  20, Qt::AlignLeft|Qt::AlignBottom, "FlatCap" );
	pen.setColor( Qt::black );
	pen.setWidth( 15 );
	pen.setCapStyle( Qt::FlatCap );
	painter.setPen( pen );
	painter.drawLine( 150,  20, 240,  20 );

	pen.setWidth( 0 );
	pen.setColor( Qt::darkBlue );
	painter.setPen( pen );
	painter.drawText(  10,  40, 150,  20, Qt::AlignLeft|Qt::AlignBottom, "SquareCap" );
	pen.setColor( Qt::black );
	pen.setWidth( 15 );
	pen.setCapStyle( Qt::SquareCap );
	painter.setPen( pen );
	painter.drawLine( 150,  50, 240,  50 );

	pen.setWidth( 0 );
	pen.setColor( Qt::darkBlue );
	painter.setPen( pen );
	painter.drawText(  10,  70, 150,  20, Qt::AlignLeft|Qt::AlignBottom, "RoundCap" );
	pen.setColor( Qt::black );
	pen.setWidth( 15 );
	pen.setCapStyle( Qt::RoundCap );
	painter.setPen( pen );
	painter.drawLine( 150,  80, 240,  80 );
    }
};

int main(int argc, char **argv)
{
    QApplication app( argc, argv );
    Frame *mw = new Frame( 0 );
    app.setMainWidget( mw );
    mw->setCaption( "Pen Cap Styles" );
    mw->setBackgroundColor( Qt::white );
    mw->resize( 250, 100 );
    mw->show();
    return app.exec();
}
