#include <QtCore>

int main(int argc, char **argv)
{
    QCoreApplication ca(argc, argv);
    QFile f;
    f.open(stdin, QIODevice::ReadOnly);
    QString input;
    char buf[1024];
    while (qint64 len = f.read(buf, 1024))
        input += QByteArray(buf, len);
    f.close();
    QFile f2("fileWriterProcess.txt");
    f2.open(QIODevice::WriteOnly | QIODevice::Truncate);
    f2.write(input.toLatin1());
    f2.close();
    return 0;
}
