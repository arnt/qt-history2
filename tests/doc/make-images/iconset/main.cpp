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

	painter.drawPixmap( 0, 0, ico.pixmap( QIconSet::Small, QIconSet::Normal ) );
	painter.drawText( 30, 0, 150, 30, Qt::AlignVCenter, "Small, Normal" );

	painter.drawPixmap( 0, 30, ico.pixmap( QIconSet::Small, QIconSet::Active ) );
	painter.drawText( 30, 30, 150, 30, Qt::AlignVCenter, "Small, Active" );

	painter.drawPixmap( 0, 60, ico.pixmap( QIconSet::Small, QIconSet::Disabled ) );
	painter.drawText( 30, 60, 150, 30, Qt::AlignVCenter, "Small, Disabled" );

	painter.drawPixmap( 200, 0, ico.pixmap( QIconSet::Large, QIconSet::Normal ) );
	painter.drawText( 240, 0, 150, 30, Qt::AlignVCenter, "Large, Normal" );

	painter.drawPixmap( 200, 30, ico.pixmap( QIconSet::Large, QIconSet::Active ) );
	painter.drawText( 240, 30, 150, 30, Qt::AlignVCenter, "Large, Active" );

	painter.drawPixmap( 200, 60, ico.pixmap( QIconSet::Large, QIconSet::Disabled ) );
	painter.drawText( 240, 60, 150, 30, Qt::AlignVCenter, "Large, Disabled" );

	painter.drawText( 0, 100, 330, 30, Qt::AlignCenter, "Normal == Active unless explicitly set" );
    }
};

int main(int argc, char **argv)
{
    QApplication app( argc, argv );
    Frame *mw = new Frame( 0 );
    app.setMainWidget( mw );
    mw->setCaption( "QIconSet" );
    mw->setBackgroundColor( Qt::white );
    mw->resize( 360, 130 );
    mw->show();
    return app.exec();
}
