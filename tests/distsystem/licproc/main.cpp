#include "licprocapp.h"


int main( int argc, char** argv )
{
    LicProcApp* app = new LicProcApp( argc, argv );

    app->exec();

    return 0;
}