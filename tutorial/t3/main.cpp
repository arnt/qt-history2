//
// Qt Tutorial 3
//
//

#include <qapp.h>
#include <qpushbt.h>
#include <qfont.h>


int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    QWidget w;
    w.setGeometry( 100, 100, 200, 120 );

    QPushButton quit( "Quit!", &w );
    quit.move( 40, 40 );
    quit.resize( 120, 40 );
    quit.setFont( QFont( "Times", 18, QFont::Bold ) );

    QObject::connect( &quit, SIGNAL(clicked()), qApp, SLOT(quitApp()) );

    a.setMainWidget( &w );
    w.show();
    return a.exec();
}
