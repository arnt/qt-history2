#include <qapplication.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include "stable.h"    // Last stable header
#include "myobject.h"  // Unstable
#include "mydialog.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    
    MyObject obj;
    MyDialog dia;
    app.setMainWidget( &dia );
    dia.connect( dia.aButton, SIGNAL(clicked()), SLOT(close()) );
    dia.show();

    return app.exec();
}
