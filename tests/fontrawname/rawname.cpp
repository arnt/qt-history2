#include "rawtest.h"

main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QFont fnt( "utopia", 8, 48, FALSE );

    RawTest test;
    test.resize( 800, 200 );

    test.show();
    app.setMainWidget( &test );

    return app.exec();
}
