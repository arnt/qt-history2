/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "uic.h"
#include "option.h"
#include "driver.h"

#include <qcoreapplication.h>
#include <qfile.h>

static const char *error = 0;

void showHelp(const char *appName)
{
    fprintf(stderr, "Qt user interface compiler %s.\n", QT_VERSION_STR);
    if (error)
        fprintf(stderr, "%s: %s\n", appName, error);

    fprintf(stderr, "Usage: %s [OPTION]... <UIFILE>\n\n"
            "  -h, -help                 display this help and exit\n"
            "  -v, -version              display version\n"
            "  -o <file>                 place the output into <file>\n"
            "  -tr <func>                use func() for i18n\n"
            "  -p, -no-protection        disable header protection\n"
            "\n", appName);

}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Driver driver;

    const char *fileName = 0;

    int arg = 1;
    while (arg < argc) {
        QString opt = QString::fromUtf8(argv[arg]);
        if (opt == QLatin1String("-h") || opt == QLatin1String("-help")) {
            showHelp(argv[0]);
            return 0;
        } else if (opt == QLatin1String("-v") || opt == QLatin1String("-version")) {
            fprintf(stderr, "Qt user interface compiler %s.\n", QT_VERSION_STR);
            return 0;
        } else if (opt == QLatin1String("-o") || opt == QLatin1String("-output")) {
            ++arg;
            if (!argv[arg]) {
                showHelp(argv[0]);
                return 1;
            }
            driver.option().outputFile = QFile::decodeName(argv[arg]);
        } else if (opt == QLatin1String("-p") || opt == QLatin1String("-no-protection")) {
            driver.option().headerProtection = false;
        } else if (opt == QLatin1String("-tr") || opt == QLatin1String("-translate")) {
            ++arg;
            if (!argv[arg]) {
                showHelp(argv[0]);
                return 1;
            }
            driver.option().translateFunction = QLatin1String(argv[arg]);
        } else if (!fileName) {
            fileName = QFile::decodeName(argv[arg]);
        } else {
            showHelp(argv[0]);
            return 1;
        }

        ++arg;
    }

    QTextStream *out = 0;
    QFile f;
    if (driver.option().outputFile.size()) {
        f.setFileName(driver.option().outputFile);
        if (!f.open(IO_WriteOnly)) {
            fprintf(stderr, "Could not create output file\n");
            return 1;
        }
        out = new QTextStream(&f);
    }

    QString inputFile;
    if (fileName)
        inputFile = QString::fromUtf8(fileName);
    else
        driver.option().headerProtection = false;

    bool rtn = driver.uic(inputFile, out);
    if (!rtn)
        fprintf(stderr, "File '%s' is not valid\n", inputFile.isEmpty() ? "<stdin>" : inputFile.local8Bit());

    delete out;

    return !rtn;
}

