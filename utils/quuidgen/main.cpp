#include <qapplication.h>
#include "quuidgen.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QUuidGen dialog;
    a.setMainWidget( &dialog );

    dialog.show();

    return a.exec();
}
