#include <qapp.h>
#include <qlabel.h>
#include <qsplitter.h>


int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    QSplitter m( QSplitter::Vertical );

    //m.setFrameStyle( QFrame::Panel | QFrame::Sunken );

    QLabel l1( "Judean People's Front", &m );
    //    l1.setBackgroundColor( white );
    l1.setMinimumHeight( 150 );
    m.setFirstWidget( &l1 );

    QLabel l2( "Judean Popular People's Front", &m );
    //    l2.setBackgroundColor( white );
    l2.setAlignment( AlignCenter );
    l2.setMaximumHeight( 250 );
    l2.setMinimumHeight( 50 );
    m.setSecondWidget( &l2 );


    a.setMainWidget( &m );

    m.show();
    return a.exec();
}
