#include <qapplication.h>
#include "qtextview.h"

int main( int argc, char* argv[]  )
{
    QApplication a( argc, argv);
    QtTextEdit v;
    a.setMainWidget( &v );
    v.setText("<p>Hallo</p><p>Das ist ein <b>Test Text</b>, gut was? </p>");
    v.show();
    return a.exec();
}
