#include <qapplication.h>
#include <qlabel.h>
#include <qsplitter.h>


int main( int argc, char ** argv )
{
    QApplication a( argc, argv );


    QSplitter f( QSplitter::Vertical );

    QLabel l( "Judean Popular Front" );
    l.setBackgroundColor( blue.light() );
    l.setMinimumSize( l.sizeHint() );
    f.setFirstWidget( &l );

    QSplitter m( &f );
    f.setSecondWidget( &m );


    //m.setFrameStyle( QFrame::Panel | QFrame::Sunken );

    QLabel l1( "Judean People's Front", &m );
    //    l1.setBackgroundColor( white );
    l1.setMinimumSize( l1.sizeHint() );
    m.setFirstWidget( &l1 );

    QLabel l2( "Judean Popular People's Front", &m );
    //    l2.setBackgroundColor( white );
    l2.setAlignment( AlignCenter );

    l2.setMaximumHeight( 250 );
    l2.setMinimumSize( l2.sizeHint().width(), 100 );
    m.setSecondWidget( &l2 );


    a.setMainWidget( &f );

    f.show();
    return a.exec();
}
