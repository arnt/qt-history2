/*
    Use this to create: brush-styles.png
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

	painter.setBrush( QBrush( Qt::red, Qt::SolidPattern ) );
	painter.drawRect( 0,     0, 150, 40 );
	painter.drawText( 0,     0, 150, 40, Qt::AlignCenter, "SolidPattern" );

	painter.setBrush( QBrush( Qt::red, Qt::Dense1Pattern ) );
	painter.drawRect( 0,    40, 150, 40 );
	painter.drawText( 0,    40, 150, 40, Qt::AlignCenter, "Dense1Pattern" );

	painter.setBrush( QBrush( Qt::red, Qt::Dense2Pattern ) );
	painter.drawRect( 0,    80, 150, 40 );
	painter.drawText( 0,    80, 150, 40, Qt::AlignCenter, "Dense2Pattern" );

	painter.setBrush( QBrush( Qt::red, Qt::Dense3Pattern ) );
	painter.drawRect( 0,   120, 150, 40 );
	painter.drawText( 0,   120, 150, 40, Qt::AlignCenter, "Dense3Pattern" );

	painter.setBrush( QBrush( Qt::red, Qt::Dense4Pattern ) );
	painter.drawRect( 0,   160, 150, 40 );
	painter.drawText( 0,   160, 150, 40, Qt::AlignCenter, "Dense4Pattern" );

	painter.setBrush( QBrush( Qt::red, Qt::Dense5Pattern ) );
	painter.drawRect( 0,   200, 150, 40 );
	painter.drawText( 0,   200, 150, 40, Qt::AlignCenter, "Dense5Pattern" );

	painter.setBrush( QBrush( Qt::red, Qt::Dense6Pattern ) );
	painter.drawRect( 0,   240, 150, 40 );
	painter.drawText( 0,   240, 150, 40, Qt::AlignCenter, "Dense6Pattern" );

	painter.setBrush( QBrush( Qt::red, Qt::Dense7Pattern ) );
	painter.drawRect( 0,   280, 150, 40 );
	painter.drawText( 0,   280, 150, 40, Qt::AlignCenter, "Dense7Pattern" );

	painter.setBrush( QBrush( Qt::red, Qt::HorPattern ) );
	painter.drawRect( 150,   0, 150, 40 );
	painter.drawText( 150,   0, 150, 40, Qt::AlignCenter, "HorPattern" );

	painter.setBrush( QBrush( Qt::red, Qt::VerPattern ) );
	painter.drawRect( 150,  40, 150, 40 );
	painter.drawText( 150,  40, 150, 40, Qt::AlignCenter, "VerPattern" );

	painter.setBrush( QBrush( Qt::red, Qt::CrossPattern ) );
	painter.drawRect( 150,  80, 150, 40 );
	painter.drawText( 150,  80, 150, 40, Qt::AlignCenter, "CrossPattern" );

	painter.setBrush( QBrush( Qt::red, Qt::BDiagPattern ) );
	painter.drawRect( 150, 120, 150, 40 );
	painter.drawText( 150, 120, 150, 40, Qt::AlignCenter, "BDiagPattern" );

	painter.setBrush( QBrush( Qt::red, Qt::FDiagPattern ) );
	painter.drawRect( 150, 160, 150, 40 );
	painter.drawText( 150, 160, 150, 40, Qt::AlignCenter, "FDiagPattern" );

	painter.setBrush( QBrush( Qt::red, Qt::DiagCrossPattern ) );
	painter.drawRect( 150, 200, 150, 40 );
	painter.drawText( 150, 200, 150, 40, Qt::AlignCenter, "DiagCrossPattern" );

	painter.setBrush( Qt::NoBrush );
	painter.drawRect( 150, 240, 150, 40 );
	painter.drawText( 150, 240, 150, 40, Qt::AlignCenter, "NoBrush" );

	painter.setBrush( Qt::NoBrush );
	painter.drawRect( 150, 280, 150, 40 );
	painter.drawText( 150, 280, 150, 40, Qt::AlignCenter, "CustomPattern\n(Pixmap)" );
    }
};

int main(int argc, char **argv)
{
    QApplication app( argc, argv );
    Frame *mw = new Frame( 0 );
    app.setMainWidget( mw );
    mw->setCaption( "Brush Styles" );
    mw->setBackgroundColor( Qt::white );
    mw->resize( 300, 320 );
    mw->show();
    return app.exec();
}
