#include <QtGui>
#include <QX11EmbedContainer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (app.arguments().count() != 2) {
        qFatal("Error - expected executable path as argument");
        return 1;
    }

    QX11EmbedContainer container;
    container.show();

    QProcess process(&container);
    QString executable(app.arguments()[1]);
    QStringList arguments;
    arguments << QString::number(container.winId());
    process.start(executable, arguments);

    int status = app.exec();
    process.close();
    return status;
}
