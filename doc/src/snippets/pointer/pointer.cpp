#include <QApplication>
#include <QLabel>
#include <QPointer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QPointer<QLabel> label = new QLabel;
    label->setText("&Status:");

    if (label)
        label->show();
    return 0;
}
