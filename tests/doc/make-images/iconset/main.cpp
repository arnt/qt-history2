/*
    Use this to create: brush-styles.png
*/
#include <qapplication.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qiconset.h>
#include <qframe.h>

class Frame : public QFrame
{
public:
    Frame( QWidget *parent, const char * name = 0, WFlags f = 0 ) :
	QFrame( parent, name, f ) {}

    void paintEvent( QPaintEvent * )
    {
	QIconSet ico = QIconSet( QPixmap( "fileopen.png" ) );
	QPainter painter( this );
	painter.setPen( Qt::black );
	painter.setFont( QFont( "Helvetica", 10, QFont::Bold ) );

	painter.drawPixmap( 0, 0, ico.pixmap( QIconSet::Small, QIconSet::Normal, QIconSet::On ) );
	painter.drawText( 25, 0, 150, 30, Qt::AlignCenter, "Small, Normal, On" );

	painter.drawPixmap( 0, 30, ico.pixmap( QIconSet::Small, QIconSet::Normal, QIconSet::Off ) );
	painter.drawText( 25, 30, 150, 30, Qt::AlignCenter, "Small, Normal, Off" );

	painter.drawPixmap( 0, 60, ico.pixmap( QIconSet::Small, QIconSet::Active, QIconSet::On ) );
	painter.drawText( 25, 60, 150, 30, Qt::AlignCenter, "Small, Active, On" );

	painter.drawPixmap( 0, 90, ico.pixmap( QIconSet::Small, QIconSet::Active, QIconSet::Off ) );
	painter.drawText( 25, 90, 150, 30, Qt::AlignCenter, "Small, Active, Off" );

	painter.drawPixmap( 0, 120, ico.pixmap( QIconSet::Small, QIconSet::Disabled, QIconSet::On ) );
	painter.drawText( 25, 120, 150, 30, Qt::AlignCenter, "Small, Disabled, On" );

	painter.drawPixmap( 0, 150, ico.pixmap( QIconSet::Small, QIconSet::Disabled, QIconSet::Off ) );
	painter.drawText( 25, 150, 150, 30, Qt::AlignCenter, "Small, Disabled, Off" );

	painter.drawPixmap( 200, 0, ico.pixmap( QIconSet::Large, QIconSet::Normal, QIconSet::On ) );
	painter.drawText( 225, 0, 150, 30, Qt::AlignCenter, "Large, Normal, On" );

	painter.drawPixmap( 200, 30, ico.pixmap( QIconSet::Large, QIconSet::Normal, QIconSet::Off ) );
	painter.drawText( 225, 30, 150, 30, Qt::AlignCenter, "Large, Normal, Off" );

	painter.drawPixmap( 200, 60, ico.pixmap( QIconSet::Large, QIconSet::Active, QIconSet::On ) );
	painter.drawText( 225, 60, 150, 30, Qt::AlignCenter, "Large, Active, On" );

	painter.drawPixmap( 200, 90, ico.pixmap( QIconSet::Large, QIconSet::Active, QIconSet::Off ) );
	painter.drawText( 225, 90, 150, 30, Qt::AlignCenter, "Large, Active, Off" );

	painter.drawPixmap( 200, 120, ico.pixmap( QIconSet::Large, QIconSet::Disabled, QIconSet::On ) );
	painter.drawText( 225, 120, 150, 30, Qt::AlignCenter, "Large, Disabled, On" );

	painter.drawPixmap( 200, 150, ico.pixmap( QIconSet::Large, QIconSet::Disabled, QIconSet::Off ) );
	painter.drawText( 225, 150, 150, 30, Qt::AlignCenter, "Large, Disabled, Off" );
    }
};

int main(int argc, char **argv)
{
    QApplication app( argc, argv );
    Frame *mw = new Frame( 0 );
    app.setMainWidget( mw );
    mw->setCaption( "IconSet" );
    mw->setBackgroundColor( Qt::white );
//    mw->resize( 300, 320 );
    mw->show();
    return app.exec();
}
