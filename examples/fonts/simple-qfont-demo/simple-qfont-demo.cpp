/* $Id$ */

#include <qapplication.h>
#include <qpushbutton.h>
#include <qfont.h>

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    QPushButton  push( "Push me", 0, "Push me" );

    QFont font( "Bavaria", 18 );          // preferred family is Bavaria
    font.setStyleHint( QFont::Serif );    // can also use any serif font

    font.setWeight( QFont::Bold );

    push.setFont( font );

    app.setMainWidget( &push );
    push.show();
    return app.exec();
}
