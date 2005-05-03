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

    QPushButton quit("Quit");
    quit.setFont(QFont("Times", 18, QFont::Bold));
    QObject::connect(&quit, SIGNAL(clicked()), &app, SLOT(quit()));

    QVBoxLayout layout;
    layout.addWidget(&quit);
    vbox.setLayout(&layout);

    vbox.show();
    return app.exec();
}
