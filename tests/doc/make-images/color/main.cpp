/*
    Use this to create: q-colors.png
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
	painter.setPen( Qt::black );
	painter.setFont( QFont( "Helvetica", 10, QFont::Bold ) );

	painter.setBrush( QBrush( Qt::white ) );
	painter.drawRect( 0,     0, 150, 40 );
	painter.drawText( 0,     0, 150, 40, Qt::AlignCenter, "white" );

	painter.setBrush( QBrush( Qt::red ) );
	painter.drawRect( 0,    40, 150, 40 );
	painter.drawText( 0,    40, 150, 40, Qt::AlignCenter, "red" );

	painter.setBrush( QBrush( Qt::green ) );
	painter.drawRect( 0,    80, 150, 40 );
	painter.drawText( 0,    80, 150, 40, Qt::AlignCenter, "green" );

	painter.setBrush( QBrush( Qt::blue ) );
	painter.drawRect( 0,   120, 150, 40 );
	painter.drawText( 0,   120, 150, 40, Qt::AlignCenter, "blue" );

	painter.setBrush( QBrush( Qt::cyan ) );
	painter.drawRect( 0,   160, 150, 40 );
	painter.drawText( 0,   160, 150, 40, Qt::AlignCenter, "cyan" );

	painter.setBrush( QBrush( Qt::magenta ) );
	painter.drawRect( 0,   200, 150, 40 );
	painter.drawText( 0,   200, 150, 40, Qt::AlignCenter, "magenta" );

	painter.setBrush( QBrush( Qt::yellow ) );
	painter.drawRect( 0,   240, 150, 40 );
	painter.drawText( 0,   240, 150, 40, Qt::AlignCenter, "yellow" );

	painter.setBrush( QBrush( Qt::gray ) );
	painter.drawRect( 0,   280, 150, 40 );
	painter.drawText( 0,   280, 150, 40, Qt::AlignCenter, "gray" );

	painter.setBrush( QBrush( Qt::lightGray ) );
	painter.drawRect( 0,   320, 300, 40 );
	painter.drawText( 0,   320, 300, 40, Qt::AlignCenter, "lightGray" );



	painter.setBrush( QBrush( Qt::black ) );
	painter.drawRect( 150,   0, 150, 40 );
	painter.setPen( Qt::white );
	painter.drawText( 150,   0, 150, 40, Qt::AlignCenter, "black" );
	painter.setPen( Qt::black );

	painter.setBrush( QBrush( Qt::darkRed ) );
	painter.drawRect( 150,  40, 150, 40 );
	painter.drawText( 150,  40, 150, 40, Qt::AlignCenter, "darkRed" );

	painter.setBrush( QBrush( Qt::darkGreen ) );
	painter.drawRect( 150,  80, 150, 40 );
	painter.drawText( 150,  80, 150, 40, Qt::AlignCenter, "darkGreen" );

	painter.setBrush( QBrush( Qt::darkBlue ) );
	painter.drawRect( 150, 120, 150, 40 );
	painter.drawText( 150, 120, 150, 40, Qt::AlignCenter, "darkBlue" );

	painter.setBrush( QBrush( Qt::darkCyan ) );
	painter.drawRect( 150, 160, 150, 40 );
	painter.drawText( 150, 160, 150, 40, Qt::AlignCenter, "darkCyan" );

	painter.setBrush( QBrush( Qt::darkMagenta ) );
	painter.drawRect( 150, 200, 150, 40 );
	painter.drawText( 150, 200, 150, 40, Qt::AlignCenter, "darkMagenta" );

	painter.setBrush( QBrush( Qt::darkYellow ) );
	painter.drawRect( 150, 240, 150, 40 );
	painter.drawText( 150, 240, 150, 40, Qt::AlignCenter, "darkYellow" );

	painter.setBrush( QBrush( Qt::darkGray ) );
	painter.drawRect( 150, 280, 150, 40 );
	painter.drawText( 150, 280, 150, 40, Qt::AlignCenter, "darkGray" );

    }
};

int main(int argc, char **argv)
{
    QApplication app( argc, argv );
    Frame *mw = new Frame( 0 );
    app.setMainWidget( mw );
    mw->setCaption( "Qt Colors" );
    mw->setBackgroundColor( Qt::white );
    mw->resize( 300, 360 );
    mw->show();
    return app.exec();
}
