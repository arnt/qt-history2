#include <qapplication.h>
#include "dumbterm.h"

main(int argc, char** argv)
{
    QApplication app(argc,argv);
    DumbTerminal *m = new DumbTerminal;
    app.setMainWidget(m);
    m->adjustSize();
    m->show();
    app.exec();
    delete m;
}
