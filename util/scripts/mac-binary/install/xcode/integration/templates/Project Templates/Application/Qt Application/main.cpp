#include <QtGui/QtGui>

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    QLabel label;
    label.setText("Hello world!");
    a.setMainWidget(&label);
    label.show();

    return a.exec();
}
