#include <qapplication.h>
#include <qlabel.h>
#include <qsplitter.h>


int main( int argc, char ** argv )
{
    QApplication a( argc, argv );


    QSplitter f( QSplitter::Vertical );

    QLabel l( "Judean Popular Front", &f );
    l.setBackgroundColor( Qt::blue.light() );
    l.setMinimumSize( l.sizeHint() );


    QSplitter m( &f );



    //m.setFrameStyle( QFrame::Panel | QFrame::Sunken );

    QLabel l1( "Judean People's Front", &m );
    //    l1.setBackgroundColor( white );
    l1.setMinimumSize( l1.sizeHint() );


    QLabel l2( "Judean Popular People's Front", &m );
    //    l2.setBackgroundColor( white );
    l2.setAlignment( Qt::AlignCenter );

    l2.setMaximumHeight( 250 );
    l2.setMinimumSize( l2.sizeHint().width(), 100 );



    a.setMainWidget( &f );

    f.show();
    return a.exec();
}
