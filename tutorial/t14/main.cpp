/****************************************************************
**
** Qt tutorial 14
**
****************************************************************/

#include <QApplication>

#include "gameboard.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    GameBoard board;
    board.setGeometry(100, 100, 500, 355);
    board.show();
    return app.exec();
}
