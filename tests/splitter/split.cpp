#include <qapplication.h>
#include <qlabel.h>
#include <qsplitter.h>

#include <qlineedit.h>

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );


    QSplitter f( QSplitter::Vertical );

    
    QSplitter top( &f );
    
    QLabel l( "Label", &top );
    l.setBackgroundColor( Qt::blue.light() );
    
    QLineEdit led( &top );
    top.setResizeMode( &l, QSplitter::FollowSizeHint );
    
    
    QObject::connect( &led, SIGNAL(textChanged(const QString&)),
		      &l, SLOT(setText(const QString&)) );

    QSplitter m( &f );



    //m.setFrameStyle( QFrame::Panel | QFrame::Sunken );

    QLabel l1( "Judean People's Front", &m );
    //    l1.setBackgroundColor( white );
    l1.setMinimumSize( l1.sizeHint() );


    QLabel l2( "Judean Popular Front", &m );
    //    l2.setBackgroundColor( white );
    l2.setAlignment( Qt::AlignCenter );

    l2.setMaximumHeight( 250 );
    l2.setMinimumSize( l2.sizeHint().width(), 100 );



    a.setMainWidget( &f );

    f.show();
    return a.exec();
}
