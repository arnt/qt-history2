#include <QApplication>

#include "../connection.h"
#include "tableeditor.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    createConnection();
    TableEditor editor("persons");
    app.setMainWidget(&editor);
    editor.show();
    return app.exec();
}
