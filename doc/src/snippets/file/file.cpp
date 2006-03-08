#include <QFile>
#include <QTextStream>

static void process_line(const QByteArray &)
{
}

static void process_line(const QString &)
{
}

static void noStream_snippet()
{
    QFile file("in.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        process_line(line);
    }
}

static void readTextStream_snippet()
{
    QFile file("in.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        process_line(line);
    }
}

static void writeTextStream_snippet()
{
    QFile file("out.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << "The magic number is: " << 49 << "\n";
}

static void writeTextStream_snippet()
{
    QFile file("out.dat");
    if (!file.open(QIODevice::WriteOnly))
        return;

    QDataStream out(&file);
    out << "The magic number is: " << 49 << "\n";
}

static void readRegularEmptyFile_snippet()
{
    QFile file("/proc/modules");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    QString line = in.readLine();
    while (!line.isNull()) {
        process_line(line);
        line = in.readLine();
    }
}

int main()
{
    lineByLine_snippet();
    writeStream_snippet();
}
