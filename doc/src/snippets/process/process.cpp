#include <QProcess>

bool zip()
{
    QProcess gzip;
    gzip.start("gzip", QStringList() << "-c");
    if (!gzip.waitForStarted())
        return false;

    gzip.write("Qt rocks!");
    gzip.closeOutputChannel();

    if (!gzip.waitForFinished())
        return false;

    QByteArray result = gzip.readAll();

    gzip.start("gzip", QStringList() << "-d" << "-c");
    gzip.write(result);
    gzip.closeOutputChannel();

    if (!gzip.waitForFinished())
        return false;

    qDebug("Result: %s", gzip.readAll().data());
    return true;
}


int main()
{
    zip();
    return 0;
}
