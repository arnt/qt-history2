// Default includes
#include "qapplication.h"

// Test specific include
#include "tst_qrtstring.h"

int main(int argc, char* argv[])
{
    // Create the test case
    tst_QRTString src( argc, argv );

    // And finally run the required tests
    return src.exec();
}
