/*
    Use this to create: pen-join-styles.png
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

	painter.drawText(  10,  10, 100,  50, Qt::AlignCenter, "MiterJoin" );
	pen.setColor( Qt::black );
	pen.setWidth( 15 );
	pen.setJoinStyle( Qt::MiterJoin );
	painter.setPen( pen );
	painter.drawRect(  10,  10, 100,  50 );

	pen.setWidth( 0 );
	pen.setColor( Qt::darkBlue );
	painter.setPen( pen );
	painter.drawText( 130,  10, 100,  50, Qt::AlignCenter, "BevelJoin" );
	pen.setColor( Qt::black );
	pen.setWidth( 15 );
	pen.setJoinStyle( Qt::BevelJoin );
	painter.setPen( pen );
	painter.drawRect( 130,  10, 100,  50 );

	pen.setWidth( 0 );
	pen.setColor( Qt::darkBlue );
	painter.setPen( pen );
	painter.drawText( 250,  10, 100,  50, Qt::AlignCenter, "RoundJoin" );
	pen.setColor( Qt::black );
	pen.setWidth( 15 );
	pen.setJoinStyle( Qt::RoundJoin );
	painter.setPen( pen );
	painter.drawRect( 250,  10, 100,  50 );

    }
};

int main(int argc, char **argv)
{
    QApplication app( argc, argv );
    Frame *mw = new Frame( 0 );
    app.setMainWidget( mw );
    mw->setCaption( "Pen Join Styles" );
    mw->setBackgroundColor( Qt::white );
    mw->resize( 360, 70 );
    mw->show();
    return app.exec();
}
