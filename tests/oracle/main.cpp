#include "qapplication.h"
#include "oramonitorimpl.h"

int main( int argc, char** argv )
{
    QApplication* app = new QApplication( argc, argv );
    OraMonitorImpl* gui = new OraMonitorImpl;

    gui->exec();

    return 0;
}
