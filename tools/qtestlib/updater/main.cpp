#include <QtCore/QtCore>

#include <stdio.h>

void printHelp(char *argv[])
{
    qDebug("usage: %s [-diff] FILES", argv[0] ? argv[0] : "qtest2to4");
    qDebug("updates FILES from QtTestLib 2.x to QTestLib 4.1");
    exit(2);
}

int main(int argc, char *argv[])
{
    bool printDiff = false;
    int i = 1;

    if (argc == 1)
        printHelp(argv);
    if (argv[1][0] == '-') {
        if (qstrcmp(argv[1], "-diff") == 0) {
            printDiff = true;
            ++i;
        } else {
            qDebug("Unknown option: %s\n", argv[1]);
            printHelp(argv);
        }
    }

    QRegExp dataHeaderRx(QLatin1String("_data(\\s*)\\((\\s*)QtTestTable\\s*\\&\\s*\\w*\\s*\\)"));
    QRegExp defElemRx(QLatin1String("\\w+\\.defineElement\\s*\\(\\s*\"(\\w+)\"\\s*,\\s*\"(\\w+)\"\\s*\\)"));

    for (; i < argc; ++i) {
        QFile f(QString::fromLocal8Bit(argv[i]));
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
            qFatal("Unable to open file '%s' for reading: %s", argv[i], qPrintable(f.errorString()));

        if (printDiff)
            printf("diff %s\n", argv[i]);

        QStringList contents;
        int lineNumber = 0;
        while (!f.atEnd()) {
            QString origLine = QString::fromLocal8Bit(f.readLine());
            QString line = origLine;
            ++lineNumber;

            if (dataHeaderRx.indexIn(line) != -1) {
                QString ws = dataHeaderRx.cap(1);
                line.replace(dataHeaderRx, QString::fromLatin1("_data%1()").arg(ws));
            }
            if (defElemRx.indexIn(line) != -1) {
                QString type = defElemRx.cap(1);
                QString name = defElemRx.cap(2);
                if (type.endsWith(QLatin1Char('>')))
                    type.append(QLatin1Char(' '));
                line.replace(defElemRx, QString::fromLatin1("QTest::addColumn<%1>(\"%2\")").arg(
                               type).arg(name));
            }

            if (printDiff && line != origLine) {
                printf("%dc%d\n", lineNumber, lineNumber);
                printf("<%s", qPrintable(origLine));
                printf("---\n");
                printf(">%s", qPrintable(line));
            }

            contents.append(line);
        }
        f.close();

        if (printDiff)
            continue;

        if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
            qFatal("Unable to open file '%s' for writing: %s", argv[i], qPrintable(f.errorString()));
        foreach (QString s, contents)
            f.write(s.toLocal8Bit());
        f.close();
    }

    return 0;
}

