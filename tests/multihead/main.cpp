#include <qapplication.h>

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    QDesktopWidget *desktop = app.desktop();

    int screens = desktop->numScreens();

    if ( desktop->isVirtualDesktop() )
	qDebug( "Virtual desktop detected." );
    qDebug( "%d screens present, %d is the primary screen.\n", screens, desktop->primaryScreen() );

    for ( int i = 0; i < screens; ++i ) {
	QRect screen = desktop->screenGeometry( i );
	qDebug( "Geometry of screen %d: TopLeft = (%d,%d), BottomRight = (%d,%d)", i, screen.left(), screen.top(), screen.right(), screen.bottom() );
    }

    return 0;
}
