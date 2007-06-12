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

#include "metatranslator.h"
#include "proparser.h"
#include "qconsole.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <errno.h>

typedef QList<MetaTranslatorMessage> TML;

static void printUsage()
{
    Console::out(QCoreApplication::tr("Usage:\n"
              "    lrelease [options] project-file\n"
              "    lrelease [options] ts-files [-qm qm-file]\n"
              "Options:\n"
              "    -help  Display this information and exit\n"
              "    -compress\n"
              "           Compress the .qm files\n"
              "    -nounfinished\n"
              "           Do not include unfinished translations\n"
              "    -silent\n"
              "           Don't explain what is being done\n"
              "    -version\n"
              "           Display the version of lrelease and exit\n" ));
}

static bool loadTsFile( MetaTranslator& tor, const QString& tsFileName,
                        bool /* verbose */ )
{
    QString qmFileName = tsFileName;
    qmFileName.replace( QRegExp(QLatin1String("\\.ts$")), QLatin1String("") );
    qmFileName += QLatin1String(".qm");

    bool ok = tor.load( tsFileName );
    if ( !ok )
        qWarning("lrelease warning: For some reason, I cannot load '%s'\n",
                 qPrintable(tsFileName));
    return ok;
}

static void releaseMetaTranslator( const MetaTranslator& tor,
                                   const QString& qmFileName, bool verbose,
                                   bool ignoreUnfinished, bool trimmed )
{
    if ( verbose )
        Console::out(QCoreApplication::tr( "Updating '%1'...\n").arg(qmFileName));
    if ( !tor.release(qmFileName, verbose, ignoreUnfinished,
                      trimmed ? Translator::Stripped
                               : Translator::Everything) )
        qWarning("lrelease warning: For some reason, I cannot save '%s'\n",
                 qPrintable(qmFileName));
}

static void releaseTsFile( const QString& tsFileName, bool verbose,
                           bool ignoreUnfinished, bool trimmed )
{
    MetaTranslator tor;
    if ( loadTsFile(tor, tsFileName, verbose) ) {
        QString qmFileName = tsFileName;
        qmFileName.replace( QRegExp(QLatin1String("\\.ts$")), QLatin1String("") );
        qmFileName += QLatin1String(".qm");
        releaseMetaTranslator( tor, qmFileName, verbose, ignoreUnfinished,
                               trimmed );
    }
}

int main( int argc, char **argv )
{
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments();
    QTranslator translator;
    if (translator.load(QLatin1String("lrelease_") + QLocale::system().name()))
        app.installTranslator(&translator);

    bool verbose = true; // the default is true starting with Qt 4.2
    bool ignoreUnfinished = false;
    bool trimmed = false; // the default is false starting with Qt 4.2
    MetaTranslator tor;
    QString outputFile;
    int numFiles = 0;
    int i;

    for ( i = 1; i < argc; i++ ) {
        if ( args[i] == QLatin1String("-compress") ) {
            trimmed = true;
            continue;
	    } else if ( args[i] == QLatin1String("-nocompress") ) {
            trimmed = false;
            continue;
        } else if ( args[i] == QLatin1String("-nounfinished") ) {
            ignoreUnfinished = true;
            continue;
        } else if ( args[i] == QLatin1String("-silent") ) {
            verbose = false;
            continue;
        } else if ( args[i] == QLatin1String("-verbose") ) {
            verbose = true;
            continue;
        } else if ( args[i] == QLatin1String("-version") ) {
            Console::out(QCoreApplication::tr( "lrelease version %1\n").arg(QT_VERSION_STR) );
            return 0;
        } else if ( args[i] == QLatin1String("-qm") ) {
            if ( i == argc - 1 ) {
                printUsage();
                return 1;
            } else {
                i++;
                outputFile = args[i];
            }
        } else if ( args[i] == QLatin1String("-help") ) {
            printUsage();
            return 0;
        } else if ( args[i][0] == QLatin1Char('-') ) {
            printUsage();
            return 1;
        } else {
            numFiles++;
        }
    }

    if ( numFiles == 0 ) {
        printUsage();
        return 1;
    }

    for ( i = 1; i < argc; i++ ) {
        if ( args[i][0] == '-' || args[i] == outputFile)
            continue;

        QFile f( args[i] );
        if ( !f.open(QIODevice::ReadOnly) ) {
#if defined(_MSC_VER) && _MSC_VER >= 1400
			char buf[100];
			strerror_s(buf, sizeof(buf), errno);
			qWarning("lrelease error: Cannot open file '%s': %s\n",
                     qPrintable(args[i]), buf);
#else
            qWarning("lrelease error: Cannot open file '%s': %s\n",
                     qPrintable(args[i]), strerror(errno));
#endif
            return 1;
        }

        QTextStream t( &f );
        QString fullText = t.readAll();
        f.close();

        if ( fullText.contains(QLatin1String("<!DOCTYPE TS>")) 
            || fullText.contains(QLatin1String("urn:oasis:names:tc:xliff:document:1.1"))) {
            if ( outputFile.isEmpty() ) {
                releaseTsFile( args[i], verbose, ignoreUnfinished, trimmed );
            } else {
                loadTsFile( tor, args[i], verbose );
            }
        } else {
            QString oldDir = QDir::currentPath();
            QDir::setCurrent( QFileInfo(args[i]).path() );
            QMap<QByteArray, QStringList> varMap;
            bool ok = evaluateProFile(args[i], verbose, &varMap );
            if (ok) {
                QStringList translations = varMap.value("TRANSLATIONS");
                if (translations.isEmpty()) {
                    qWarning("lrelease warning: Met no 'TRANSLATIONS' entry in"
                             " project file '%s'\n",
                             qPrintable(args[i]) );
                } else {
                    for (QStringList::iterator it = translations.begin(); it != translations.end(); ++it) {
                        releaseTsFile(*it, verbose, ignoreUnfinished, trimmed);
                    }
                }

                QDir::setCurrent( oldDir );
            } else {
                qWarning("error: lrelease encountered project file functionality that is currently not supported.\n"
                    "You might want to consider using .ts files as input instead of a project file.\n"
                    "Try the following syntax:\n"
                    "    lrelease [options] ts-files [-qm qm-file]\n");
            }
        }
    }

    if ( !outputFile.isEmpty() )
        releaseMetaTranslator( tor, outputFile, verbose, ignoreUnfinished,
                              trimmed );

    return 0;
}
