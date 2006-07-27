#include <QApplication>
#include <QClipboard>
#include <QStringList>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QClipboard *board = QApplication::clipboard();
    return (board->text() == app.arguments().at(1)) ? 0 : 1;
}
