#include <qapplication.h>
#include "form1.h"
#include "form2.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    Form1 w1;
    Form2 w2;
    w1.show();
    w2.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
