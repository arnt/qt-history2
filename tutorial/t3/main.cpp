/****************************************************************
**
** Qt tutorial 3
**
****************************************************************/

#include <QApplication>
#include <QFont>
#include <QPushButton>
#include <QVBoxWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QVBoxWidget vbox;
    vbox.resize(200, 120);

    QPushButton quit("Quit", &vbox);
    quit.setFont(QFont("Times", 18, QFont::Bold));

    QObject::connect(&quit, SIGNAL(clicked()), &app, SLOT(quit()));

    app.setMainWidget(&vbox);
    vbox.show();
    return app.exec();
}
