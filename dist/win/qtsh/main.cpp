#include <qapplication.h>
#include "shell.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    Shell s;
    a.exec();
}
