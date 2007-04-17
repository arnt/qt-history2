#include <QCoreApplication>
#include <QDebug>
#include <QStringList>
#include <QFile>
#include <QDir>

#include <stdio.h>

#if defined(Q_OS_UNIX)
#include <sys/types.h>
#include <unistd.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#endif

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QStringList args = app.arguments();
    if (args.count() != 2) {
        fprintf(stderr, "Usage: testDetached filename.txt\n");
        return 128;
    }

    QFile f(args.at(1));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        fprintf(stderr, "Cannot open %s for writing", qPrintable(f.fileName()));
        return 1;
    }

    f.write(QDir::currentPath().toUtf8());
    f.putChar('\n');
#if defined(Q_OS_UNIX)
    f.write(QByteArray::number(getpid()));
#elif defined(Q_OS_WIN)
    f.write(QByteArray::number(quint64(GetCurrentProcessId())));
#endif
    f.putChar('\n');

    f.close();

    return 0;
}
