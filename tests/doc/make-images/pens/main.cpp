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
	painter.setPen( Qt::black );
	painter.setFont( QFont( "Helvetica", 10, QFont::Bold ) );

	painter.drawText( 150,  0, 150, 20, Qt::AlignLeft|Qt::AlignBottom, "NoPen" );
	painter.setPen( Qt::NoPen );
	painter.drawLine( 10,  10, 140, 10 );

	painter.drawText( 150, 40, 150, 20, Qt::AlignLeft|Qt::AlignBottom, "SolidLine" );
	painter.setPen( Qt::SolidLine );
	painter.drawLine( 10,  50, 140, 50 );

	painter.drawText( 150, 80, 150, 20, Qt::AlignLeft|Qt::AlignBottom, "DashLine" );
	painter.setPen( Qt::DashLine );
	painter.drawLine( 10,  90, 140, 90 );

	painter.drawText( 150, 120, 150, 20, Qt::AlignLeft|Qt::AlignBottom, "DotLine" );
	painter.setPen( Qt::DotLine );
	painter.drawLine( 10, 130, 140, 130 );

	painter.drawText( 150, 160, 150, 20, Qt::AlignLeft|Qt::AlignBottom, "DashDotLine" );
	painter.setPen( Qt::DashDotLine );
	painter.drawLine( 10, 170, 140, 170 );

	painter.drawText( 150, 200, 150, 20, Qt::AlignLeft|Qt::AlignBottom, "DashDotDotLine" );
	painter.setPen( Qt::DashDotDotLine );
	painter.drawLine( 10, 210, 140, 210 );
    }
};

int main(int argc, char **argv)
{
    QApplication app( argc, argv );
    Frame *mw = new Frame( 0 );
    app.setMainWidget( mw );
    mw->setCaption( "Pen Styles" );
    mw->setBackgroundColor( Qt::white );
    mw->resize( 270, 225 );
    mw->show();
    return app.exec();
}
