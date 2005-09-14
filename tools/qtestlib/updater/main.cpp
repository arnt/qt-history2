/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore/QtCore>

#include <stdio.h>

void printHelp(char *argv[])
{
    qDebug("Usage: %s [-diff] FILES", argv[0] ? argv[0] : "qtest2to4");
    qDebug("updates files from QtTestLib 2.x to QTestLib 4.1");
    qDebug("\noptions:\n    -diff   Don't write any changes, output differences instead.");
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
    QRegExp defElemRx(QLatin1String("\\w+\\.defineElement\\s*\\(\\s*\"(.+)\"\\s*,\\s*\"(.+)\"\\s*\\)"));
    defElemRx.setMinimal(true);
    QRegExp addDataRx(QLatin1String("\\*\\w+\\.newData(\\s*)(\\(\\s*\".*\"\\s*\\))"));
    addDataRx.setMinimal(true);
    QRegExp nsRx(QLatin1String("namespace(\\s+)QtTest"));
    QRegExp callRx(QLatin1String("QtTest(\\s*)::"));

    for (; i < argc; ++i) {
        QFile f(QString::fromLocal8Bit(argv[i]));
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
            qFatal("Unable to open file '%s' for reading: %s", argv[i], qPrintable(f.errorString()));

        if (printDiff)
            printf("diff %s\n", argv[i]);

        QStringList contents;
        int lineNumber = 0;
        int changedLines = 0;
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
            if (addDataRx.indexIn(line) != -1) {
                QString repl = QLatin1String("QTest::newRow");
                repl += addDataRx.cap(1);
                repl += addDataRx.cap(2);
                line.replace(addDataRx, repl);
            }
            if (nsRx.indexIn(line) != -1)
                line.replace(nsRx, QString::fromLatin1("namespace%1QTest").arg(nsRx.cap(1)));
            int pos = 0;
            while ((pos = callRx.indexIn(line, pos)) != -1) {
                line.replace(callRx, QString::fromLatin1("QTest%1::").arg(callRx.cap(1)));
                pos += callRx.matchedLength();
            }

            line.replace(QLatin1String("QTTEST_MAIN"), QLatin1String("QTEST_MAIN"));
            line.replace(QLatin1String("QTTEST_APPLESS_MAIN"), QLatin1String("QTEST_APPLESS_MAIN"));
            line.replace(QLatin1String("QTTEST_NOOP_MAIN"), QLatin1String("QTEST_NOOP_MAIN"));
            line.replace(QLatin1String("QtTestEventLoop"), QLatin1String("QTestEventLoop"));
            line.replace(QLatin1String("QtTestEventList"), QLatin1String("QTestEventList"));
            line.replace(QLatin1String("QtTestAccessibility"), QLatin1String("QTestAccessibility"));

            if (line != origLine) {
                if (printDiff) {
                    printf("%dc%d\n", lineNumber, lineNumber);
                    printf("<%s", qPrintable(origLine));
                    printf("---\n");
                    printf(">%s", qPrintable(line));
                }
                ++changedLines;
            }

            contents.append(line);
        }
        f.close();

        if (printDiff)
            continue;
        qDebug("%s: %d change%s made.", argv[i], changedLines, changedLines == 1 ? "" : "s");
        if (!changedLines)
            continue;

        if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
            qFatal("Unable to open file '%s' for writing: %s", argv[i], qPrintable(f.errorString()));
        foreach (QString s, contents)
            f.write(s.toLocal8Bit());
        f.close();
    }

    return 0;
}

