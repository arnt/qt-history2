#include <qapplication.h>
#include <qlabel.h>

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    QLabel label( "Hello world", 0 );
    label.show();

    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
