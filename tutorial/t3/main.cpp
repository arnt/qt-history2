/****************************************************************
**
** Qt tutorial 3
**
****************************************************************/

#include <qapp.h>
#include <qpushbt.h>
#include <qfont.h>


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QWidget w;
    w.resize( 200, 120 );

    QPushButton quit( "Quit", &w );
    quit.move( 62, 40 );
    quit.resize( 75, 30 );
    quit.setFont( QFont( "Times", 18, QFont::Bold ) );

    QObject::connect( &quit, SIGNAL(clicked()), &a, SLOT(quitApp()) );

    a.setMainWidget( &w );
    w.show();
    return a.exec();
}
