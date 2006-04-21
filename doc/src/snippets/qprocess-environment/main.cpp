#include <QtCore>

void startProcess()
{
QProcess process;
QStringList env = QProcess::systemEnvironment();
env << "TMPDIR=C:\\MyApp\\temp"; // Add an environment variable
env.replaceInStrings(QRegExp("^PATH=(.*)", Qt::CaseInsensitive), "PATH=\\1;C:\\Bin");
process.setEnvironment(env);
process.start("myapp");
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    startProcess();
    return app.exec();
}
