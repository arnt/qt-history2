/****************************************************************
**
** Qt tutorial 3
**
****************************************************************/

#include <QApplication>
#include <QFont>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget vbox;
    vbox.resize(200, 120);

    QVBoxLayout layout;
    vbox.setLayout(&layout);

    QPushButton quit("Quit");
    quit.setFont(QFont("Times", 18, QFont::Bold));
    layout.addWidget(&quit);

    QObject::connect(&quit, SIGNAL(clicked()), &app, SLOT(quit()));

    vbox.show();
    return app.exec();
}
