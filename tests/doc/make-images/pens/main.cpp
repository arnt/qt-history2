/*
    Use this to create: pen-style.png
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

	painter.setPen( Qt::darkGreen );
	painter.drawText( 150,  10, 160,  20, Qt::AlignLeft|Qt::AlignBottom, "setWidth(0)" );
	painter.drawText( 250,  10, 160,  20, Qt::AlignLeft|Qt::AlignBottom, "setWidth(2)" );
	painter.drawText( 350,  10, 160,  20, Qt::AlignLeft|Qt::AlignBottom, "setWidth(3)" );
	painter.drawText( 450,  10, 160,  20, Qt::AlignLeft|Qt::AlignBottom, "setWidth(4)" );

	painter.setPen( Qt::darkBlue );
	QPen pen = painter.pen();

	painter.drawText(  10,  40, 150,  20, Qt::AlignLeft|Qt::AlignBottom, "NoPen" );
	pen.setColor( Qt::black );
	pen.setStyle( Qt::NoPen );
	painter.setPen( pen );
	painter.drawLine( 150,  50, 240,  50 );

	pen.setWidth( 0 );
	pen.setColor( Qt::darkBlue );
	painter.setPen( pen );
	painter.drawText(  10,  70, 150,  20, Qt::AlignLeft|Qt::AlignBottom, "SolidLine" );
	pen.setColor( Qt::black );
	pen.setStyle( Qt::SolidLine );
	painter.setPen( pen );
	painter.drawLine( 150,  80, 240,  80 );
	pen.setWidth( 2 );
	painter.setPen( pen );
	painter.drawLine( 250,  80, 340,  80 );
	pen.setWidth( 3 );
	painter.setPen( pen );
	painter.drawLine( 350,  80, 440,  80 );
	pen.setWidth( 4 );
	painter.setPen( pen );
	painter.drawLine( 450,  80, 540,  80 );

	pen.setWidth( 0 );
	pen.setColor( Qt::darkBlue );
	painter.setPen( pen );
	painter.drawText(  10, 100, 150,  20, Qt::AlignLeft|Qt::AlignBottom, "DashLine" );
	pen.setColor( Qt::black );
	pen.setStyle( Qt::DashLine );
	painter.setPen( pen );
	painter.drawLine( 150, 110, 240, 110 );
	pen.setWidth( 2 );
	painter.setPen( pen );
	painter.drawLine( 250, 110, 340, 110 );
	pen.setWidth( 3 );
	painter.setPen( pen );
	painter.drawLine( 350, 110, 440, 110 );
	pen.setWidth( 4 );
	painter.setPen( pen );
	painter.drawLine( 450, 110, 540, 110 );

	pen.setWidth( 0 );
	pen.setColor( Qt::darkBlue );
	painter.setPen( pen );
	painter.drawText(  10, 130, 150,  20, Qt::AlignLeft|Qt::AlignBottom, "DotLine" );
	pen.setColor( Qt::black );
	pen.setStyle( Qt::DotLine );
	painter.setPen( pen );
	painter.drawLine( 150, 140, 240, 140 );
	pen.setWidth( 2 );
	painter.setPen( pen );
	painter.drawLine( 250, 140, 340, 140 );
	pen.setWidth( 3 );
	painter.setPen( pen );
	painter.drawLine( 350, 140, 440, 140 );
	pen.setWidth( 4 );
	painter.setPen( pen );
	painter.drawLine( 450, 140, 540, 140 );

	pen.setWidth( 0 );
	pen.setColor( Qt::darkBlue );
	painter.setPen( pen );
	painter.drawText(  10, 160, 150,  20, Qt::AlignLeft|Qt::AlignBottom, "DashDotLine" );
	pen.setColor( Qt::black );
	pen.setStyle( Qt::DashDotLine );
	painter.setPen( pen );
	painter.drawLine( 150, 170, 240, 170 );
	pen.setWidth( 2 );
	painter.setPen( pen );
	painter.drawLine( 250, 170, 340, 170 );
	pen.setWidth( 3 );
	painter.setPen( pen );
	painter.drawLine( 350, 170, 440, 170 );
	pen.setWidth( 4 );
	painter.setPen( pen );
	painter.drawLine( 450, 170, 540, 170 );

	pen.setWidth( 0 );
	pen.setColor( Qt::darkBlue );
	painter.setPen( pen );
	painter.drawText(  10, 190, 150,  20, Qt::AlignLeft|Qt::AlignBottom, "DashDotDotLine" );
	pen.setColor( Qt::black );
	pen.setStyle( Qt::DashDotDotLine );
	painter.setPen( pen );
	painter.drawLine( 150, 200, 240, 200 );
	pen.setWidth( 2 );
	painter.setPen( pen );
	painter.drawLine( 250, 200, 340, 200 );
	pen.setWidth( 3 );
	painter.setPen( pen );
	painter.drawLine( 350, 200, 440, 200 );
	pen.setWidth( 4 );
	painter.setPen( pen );
	painter.drawLine( 450, 200, 540, 200 );
    }
};

int main(int argc, char **argv)
{
    QApplication app( argc, argv );
    Frame *mw = new Frame( 0 );
    app.setMainWidget( mw );
    mw->setCaption( "Pen Styles" );
    mw->setBackgroundColor( Qt::white );
    mw->resize( 550, 220 );
    mw->show();
    return app.exec();
}
