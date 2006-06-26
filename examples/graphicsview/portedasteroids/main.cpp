#include <qapplication.h>
#include "toplevel.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    KAstTopLevel topLevel;
    topLevel.setWindowTitle("Ported Asteroids Game");
    topLevel.show();

    app.setQuitOnLastWindowClosed(true);
    return app.exec();
}
