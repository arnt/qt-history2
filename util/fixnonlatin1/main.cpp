#include <QtCore/QtCore>

// Scans files for characters >127 and replaces them with the \nnn octal representation

int main(int argc, char *argv[])
{
    if (argc <= 1)
        qFatal("Usage: %s FILES", argc ? argv[0] : "fixnonlatin1");
    for (int i = 1; i < argc; ++i) {
        QFile file(QString::fromLocal8Bit(argv[i]));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            qFatal("Cannot open '%s': %s", argv[i], qPrintable(file.errorString()));

        QByteArray ba = file.readAll();
        bool mod = false;
        for (int j = 0; j < ba.count(); ++j) {
            uchar c = ba.at(j);
            if (c > 127) {
                ba[j] = '\\';
                ba.insert(j + 1, QByteArray::number(c, 8).rightJustified(3, '0', true));
                j += 3;
                mod = true;
            }
        }
        file.close();

        qDebug("found non-latin1 characters in '%s'", argv[i]);
        if (!mod)
            continue;

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            qFatal("Cannot open '%s' for writing: %s", argv[i], qPrintable(file.errorString()));
        if (file.write(ba) < 0)
            qFatal("Error while writing into '%s': %s", argv[i], qPrintable(file.errorString()));
        file.close();
    }

    return 0;
}

