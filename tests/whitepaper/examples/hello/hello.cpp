/*
  hello.cpp
*/

// quote
#include <qapplication.h>
#include <qlabel.h>

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    QLabel hello( "<font color=blue>Hello <i>Qt!</i></font>", 0 );
    app.setMainWidget( &hello );
    hello.show();
    return app.exec();
}
// endquote
