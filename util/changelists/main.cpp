#include <qapplication.h>

#include "mainform.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    MainForm mf(argc == 2 && !strcmp(argv[1], "-group"));
    mf.show();
    a.setMainWidget( &mf );
    
    return a.exec();
}
