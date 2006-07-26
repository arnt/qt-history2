#include <QApplication>
#include <QClipBoard>
#include <QStringList>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QClipboard *board = QApplication::clipboard();
    board->setText(app.arguments().at(1));
    return 0;
}
