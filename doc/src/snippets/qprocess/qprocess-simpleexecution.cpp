#include <QApplication>
#include <QtGui>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QObject *parent;
    parent = &app;

    QString program = "./path/to/Qt/examples/widgets/analogclock";
    program = "./../../../../examples/widgets/analogclock/analogclock";

    QStringList arguments;
    arguments << "-style" << "motif";

    QProcess *myProcess = new QProcess(parent);
    myProcess->start(program, arguments);

    return app.exec();
}
