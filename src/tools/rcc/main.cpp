/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "rcc.h"
#include "../../corelib/kernel/qcorecmdlineargs_p.h"

#include <QFile>
#include <QDir>

// Some static globals
static bool writeBinary = false;
static QString initName;
static bool verbose = false;
static int compressLevel = CONSTANT_COMPRESSLEVEL_DEFAULT;
static int compressThreshold = CONSTANT_COMPRESSTHRESHOLD_DEFAULT;
static QString resourceRoot;

bool processResourceFile(const QStringList &filenamesIn, const QString &filenameOut, bool list)
{
    if (verbose)
        fprintf(stderr, "Qt resource compiler\n");

    //setup
    RCCResourceLibrary library;
    library.setFormat(writeBinary ? RCCResourceLibrary::Binary : RCCResourceLibrary::C_Code);
    library.setInputFiles(filenamesIn);
    library.setInitName(initName);
    library.setVerbose(verbose);
    library.setCompressLevel(compressLevel);
    library.setCompressThreshold(compressThreshold);
    library.setResourceRoot(resourceRoot);
    if(!library.readFiles(list))
        return false;

    //open output
    FILE *out_fd = stdout;
    if (!filenameOut.isEmpty() && filenameOut != QLatin1String("-")) {
#if defined(_MSC_VER) && _MSC_VER >= 1400
        if (fopen_s(&out_fd, filenameOut.toLocal8Bit().constData(), writeBinary ? "wb": "w")) {
#else
        if(!(out_fd = fopen(filenameOut.toLocal8Bit().constData(), writeBinary ? "wb": "w"))) {
#endif
            fprintf(stderr, "Unable to open %s for writing\n", filenameOut.toLocal8Bit().constData());
            return false;
        }
    }

    //do the task
    bool ret = true;
    if(list) {
        const QStringList data = library.dataFiles();
        for(int i = 0; i < data.size(); ++i)
            fprintf(out_fd, "%s\n", QDir::cleanPath(data.at(i)).toLatin1().constData());
    } else {
        ret = library.output(out_fd);
    }
    if(out_fd != stdout)
        fclose(out_fd);

    //done
    return ret;
}

int showHelp(const QString &argv0, const QString &error)
{
    fprintf(stderr, "Qt resource compiler\n");
    if (!error.isEmpty())
        fprintf(stderr, "%s: %s\n", argv0.toLocal8Bit().constData(), error.toLocal8Bit().constData());
    fprintf(stderr, "Usage: %s  [options] <inputs>\n\n"
            "Options:\n"
            "  -o file           write output to file rather than stdout\n"
            "  -name name        create an external initialization function with name\n"
            "  -threshold level  threshold to consider compressing files\n"
            "  -compress level   compress input files by level\n"
            "  -root path        prefix resource access path with root path\n"
            "  -no-compress      disable all compression\n"
            "  -binary           output a binary file for use as a dynamic resource\n"
            "  -version          display version\n"
            "  -help             display this information\n",
            argv0.toLocal8Bit().constData());
    return 1;
}


int main(int argc, char *argv[])
{
    QString outFilename;
    bool helpRequested = false, list = false;
    QStringList files;

	QStringList args = qCmdLineArgs(argc, argv);

    //parse options
    QString errorMsg;
    for (int i = 1; i < args.count() && errorMsg.isEmpty(); i++) {
		if (args[i].isEmpty())
			continue;
        if (args[i][0] == '-') {   // option
            QByteArray opt = argv[i] + 1;
            if (opt == "o") {
                if (!(i < argc-1)) {
                    errorMsg = QLatin1String("Missing output name");
                    break;
                }
                outFilename = args[++i];
            } else if(opt == "name") {
                if (!(i < argc-1)) {
                    errorMsg = QLatin1String("Missing target name");
                    break;
                }
                initName = args[++i];
            } else if(opt == "root") {
                if (!(i < argc-1)) {
                    errorMsg = QLatin1String("Missing root path");
                    break;
                }
                resourceRoot = QDir::cleanPath(QString::fromLocal8Bit(argv[++i]));
                if(resourceRoot.isEmpty() || resourceRoot.at(0) != QLatin1Char('/'))
                    errorMsg = QLatin1String("Root must start with a /");
            } else if(opt == "compress") {
                if (!(i < argc-1)) {
                    errorMsg = QLatin1String("Missing compression level");
                    break;
                }
                compressLevel = args[++i].toInt();
            } else if(opt == "threshold") {
                if (!(i < argc-1)) {
                    errorMsg = QLatin1String("Missing compression threshold");
                    break;
                }
                compressThreshold = QString::fromLocal8Bit(argv[++i]).toInt();
            } else if(opt == "binary") {
                writeBinary = true;
            } else if(opt == "verbose") {
                verbose = true;
            } else if(opt == "list") {
                list = true;
            } else if(opt == "version" || opt == "v") {
                fprintf(stderr, "Qt Resource Compiler version %s\n", QT_VERSION_STR);
                return 1;
            } else if(opt == "help" || opt == "h") {
                helpRequested = true;
            } else if(opt == "no-compress") {
                compressLevel = -2;
            } else {
                errorMsg = QString::fromLatin1("Unknown option: '%1'").arg(args[i]);
            }
        } else {
			if(!QFile::exists(args[i])) {
                qWarning("%s: File does not exist '%s'", qPrintable(args[0]), qPrintable(args[i]));
                return 1;
            }
            files.append(args[i]);
        }
    }

    if (!files.size() || !errorMsg.isEmpty() || helpRequested)
        return showHelp(args[0], errorMsg);
    return int(!processResourceFile(files, outFilename, list));
}
