#include <qapplication.h>

#include "mainform.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    MainForm mf;

    mf.show();
    a.setMainWidget( &mf );
    
    return a.exec();
}
