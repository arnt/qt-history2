/*
$Id$
*/

#include "productlist.h"

#include <qapplication.h>

int main( int argc, char ** argv )
{
    QApplication app( argc, argv );

    ProductList * productlist = new ProductList();

    app.setMainWidget( productlist );
    productlist->show();
    return app.exec();
}
