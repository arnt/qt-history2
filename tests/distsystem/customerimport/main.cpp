#include "app.h"

int main( int argc, char** argv )
{
    ImportApp app( argc, argv );

    app.doImport();

    return 0;
}
