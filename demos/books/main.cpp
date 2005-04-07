#include <QtGui>

#include "bookwindow.h"

int main(int argc, char * argv[])
{
    QApplication app(argc, argv);

    BookWindow win;
    win.show();

    return app.exec();
}

