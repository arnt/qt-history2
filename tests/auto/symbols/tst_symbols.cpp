/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore/QtCore>
#include <QtTest/QtTest>

class tst_Symbols: public QObject
{
    Q_OBJECT
private slots:
    void prefix();
};

void tst_Symbols::prefix()
{
#if defined(Q_OS_LINUX)
    QStringList qtTypes;
    qtTypes << "QString" << "QChar" << "QWidget" << "QObject" << "QVariant" << "QList"
            << "QMap" << "QHash" << "QVector" << "QRect" << "QSize" << "QPoint"
            << "QTextFormat" << "QTextLength" << "QPen" << "QFont" << "QIcon"
            << "QPixmap" << "QImage" << "QRegion" << "QPolygon";
    QStringList qAlgorithmFunctions;
    qAlgorithmFunctions << "qBinaryFind" << "qLowerBound" << "qUpperBound"
                        << "qAbs" << "qMin" << "qMax" << "qBound" << "qSwap"
                        << "qHash" << "qDeleteAll" << "qCopy" << "qSort";

    QStringList exceptionalSymbols;
    exceptionalSymbols << "XRectangle::~XRectangle()"
                       << "XChar2b::~XChar2b()"
                       << "XPoint::~XPoint()"
                       << "glyph_metrics_t::"; // #### Qt 4.2

    QStringList stupidCSymbols;
    stupidCSymbols << "Add_Glyph_Property"
                   << "Check_Property"
                   << "Coverage_Index"
                   << "Get_Class"
                   << "Get_Device"
                   << "rcsid3"
                   << "sfnt_module_class"
                   << "t1cid_driver_class"
                   << "t42_driver_class"
                   << "winfnt_driver_class"
                   << "pshinter_module_class"
                   << "psnames_module_class"
                   ;

    QStringList excusedPrefixes;
    excusedPrefixes << "sqlite3"
                    << "ftglue_"
                    << "Load_"
                    << "otl_"
                    << "TT_"
                    << "tt_"
                    << "t1_"
                    << "Free_"
                    << "FT_"
                    << "FTC_"
                    << "ft_"
                    << "ftc_"
                    << "af_autofitter"
                    << "af_dummy"
                    << "af_latin"
                    << "autofit_"
                    << "XPanorami"
                    << "Xinerama"
                    << "bdf_"
                    << "ccf_"
                    << "gray_raster"
                    << "pcf_"
                    << "cff_"
                    << "otv_"
                    << "pfr_"
                    << "ps_"
                    << "psaux"
                    << "png_"
                    << "hb_"
                    << "HB_"
                    << "Ui_Q" // uic generated, like Ui_QPrintDialog
                    ;

    QDir dir(qgetenv("QTDIR") + "/lib", "*.so");
    QStringList files = dir.entryList();
    QVERIFY(!files.isEmpty());

    bool isFailed = false;
    foreach (QString lib, files) {
        if (lib.contains("Designer"))
            continue;

        QProcess proc;
        proc.start("nm",
           QStringList() << "-g" << "-C" << "--format=posix"
                         << "--defined-only" << dir.absolutePath() + "/" + lib);
        QVERIFY(proc.waitForFinished());
        QCOMPARE(proc.exitCode(), 0);
        QCOMPARE(QString::fromLocal8Bit(proc.readAllStandardError()), QString());

        QStringList symbols = QString::fromLocal8Bit(proc.readAll()).split("\n");
        QVERIFY(!symbols.isEmpty());
        foreach (QString symbol, symbols) {
            if (symbol.isEmpty())
                continue;
            if (symbol.startsWith("const ") || symbol.startsWith("unsigned "))
                // strip modifiers
                symbol = symbol.mid(symbol.indexOf(' ') + 1);
            if (symbol.startsWith("bool ") || symbol.startsWith("char* ")
                || symbol.startsWith("int ") || symbol.startsWith("char ")
                || symbol.startsWith("short")
                || symbol.startsWith("void ") || symbol.startsWith("void* ")) {
                // partial templates have the return type in their demangled name, strip it
                symbol = symbol.mid(symbol.indexOf(' ') + 1);
            }
            if (symbol.startsWith("_") || symbol.startsWith("std::"))
                continue;
            if (symbol.startsWith("vtable "))
                continue;
            if (symbol.startsWith("typeinfo "))
                continue;
            if (symbol.startsWith("non-virtual "))
                continue;
            if (symbol.startsWith("operator"))
                continue;
            if (symbol.startsWith("guard variable for "))
                continue;
            if (symbol.contains("(QTextStream") || symbol.contains("(Q3TextStream"))
                // QTextStream is excused.
                continue;
            if (symbol.startsWith("bitBlt") || symbol.startsWith("copyBlt"))
                // you're excused, too
                continue;

            bool symbolOk = false;

            foreach (QString prefix, excusedPrefixes)
                if (symbol.startsWith(prefix)) {
                    symbolOk = true;
                    break;
                }

            if (symbolOk)
                continue;

            foreach (QString cSymbolPattern, stupidCSymbols)
                if (symbol.contains(cSymbolPattern)) {
                    symbolOk = true;
                    break;
                }

            if (symbolOk)
                continue;

            QStringList fields = symbol.split(' ');
            // the last two fields are address and size and the third last field is the symbol type
            QVERIFY(fields.count() > 3);
            QString type = fields.at(fields.count() - 3);
            // weak symbol
            if (type == QLatin1String("W")) {
                if (symbol.contains("qAtomic"))
                    continue;

                if (symbol.contains("fstat")
                    || symbol.contains("lstat")
                    || symbol.contains("stat64")
                   )
                    continue;

                foreach (QString acceptedPattern, qAlgorithmFunctions + exceptionalSymbols)
                    if (symbol.contains(acceptedPattern)) {
                        symbolOk = true;
                        break;
                    }

                if (symbolOk)
                    continue;

                QString plainSymbol;
                for (int i = 0; i < fields.count() - 3; ++i) {
                    if (i > 0)
                        plainSymbol += QLatin1Char(' ');
                    plainSymbol += fields.at(i);
                }
                foreach (QString qtType, qtTypes)
                    if (plainSymbol.contains(qtType)) {
                        symbolOk = true;
                        break;
                    }

                if (symbolOk)
                    continue;
            }

            if (!symbol.startsWith("q", Qt::CaseInsensitive)) {
                qDebug("symbol in '%s' does not start with q: '%s'", qPrintable(lib),
                        qPrintable(symbol));
                isFailed = true;
            }
        }
    }

    if (isFailed)
        QFAIL("Libraries contain non-prefixed symbols. See Debug output :)");
#else
    QSKIP("Linux-specific test", SkipAll);
#endif
}

QTEST_MAIN(tst_Symbols)
#include "tst_symbols.moc"
