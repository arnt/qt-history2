/****************************************************************
**
** Qt tutorial 3
**
****************************************************************/

#include <qapplication.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qlayout.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QWidget w;
    w.resize( 200, 120 );

    QPushButton quit( "Quit", &w );
    quit.setFont( QFont( "Times", 18, QFont::Bold ) );

    QObject::connect( &quit, SIGNAL(clicked()), &a, SLOT(quit()) );

    QHBoxLayout layout( &w );
    layout.addWidget( &quit, 0, Qt::AlignCenter );
    
    a.setMainWidget( &w );
    w.show();
    return a.exec();
}
