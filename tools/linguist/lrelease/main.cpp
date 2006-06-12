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

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include <errno.h>

typedef QList<MetaTranslatorMessage> TML;

static void printUsage()
{
    fprintf( stderr, "Usage:\n"
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
              "           Display the version of lrelease and exit\n" );
}

static bool loadTsFile( MetaTranslator& tor, const QString& tsFileName,
                        bool /* verbose */ )
{
    QString qmFileName = tsFileName;
    qmFileName.replace( QRegExp("\\.ts$"), "" );
    qmFileName += ".qm";

    bool ok = tor.load( tsFileName );
    if ( !ok )
        fprintf( stderr,
                 "lrelease warning: For some reason, I cannot load '%s'\n",
                 tsFileName.toLatin1().data() );
    return ok;
}

static void releaseMetaTranslator( const MetaTranslator& tor,
                                   const QString& qmFileName, bool verbose,
                                   bool ignoreUnfinished, bool trimmed )
{
    if ( verbose )
        fprintf( stderr, "Updating '%s'...\n", qmFileName.toLatin1().constData() );
    if ( !tor.release(qmFileName, verbose, ignoreUnfinished,
                      trimmed ? Translator::Stripped
                               : Translator::Everything) )
        fprintf( stderr,
                 "lrelease warning: For some reason, I cannot save '%s'\n",
                 qmFileName.toLatin1().constData() );
}

static void releaseTsFile( const QString& tsFileName, bool verbose,
                           bool ignoreUnfinished, bool trimmed )
{
    MetaTranslator tor;
    if ( loadTsFile(tor, tsFileName, verbose) ) {
        QString qmFileName = tsFileName;
        qmFileName.replace( QRegExp("\\.ts$"), "" );
        qmFileName += ".qm";
        releaseMetaTranslator( tor, qmFileName, verbose, ignoreUnfinished,
                               trimmed );
    }
}

int main( int argc, char **argv )
{
    bool verbose = true; // the default is true starting with Qt 4.2
    bool ignoreUnfinished = false;
    bool trimmed = false; // the default is false starting with Qt 4.2
    bool metTranslations = false;
    MetaTranslator tor;
    QString outputFile;
    int numFiles = 0;
    int i;

    for ( i = 1; i < argc; i++ ) {
        if ( qstrcmp(argv[i], "-compress") == 0 ) {
            trimmed = true;
            continue;
	} if ( qstrcmp(argv[i], "-nocompress") == 0 ) {
            trimmed = false;
            continue;
        } else if ( qstrcmp(argv[i], "-nounfinished") == 0 ) {
            ignoreUnfinished = true;
            continue;
        } else if ( qstrcmp(argv[i], "-silent") == 0 ) {
            verbose = false;
            continue;
        } else if ( qstrcmp(argv[i], "-verbose") == 0 ) {
            verbose = true;
            continue;
        } else if ( qstrcmp(argv[i], "-version") == 0 ) {
            fprintf( stderr, "lrelease version %s\n", QT_VERSION_STR );
            return 0;
        } else if ( qstrcmp(argv[i], "-qm") == 0 ) {
            if ( i == argc - 1 ) {
                printUsage();
                return 1;
            } else {
                i++;
                outputFile = argv[i];
                argv[i][0] = '-';
            }
        } else if ( qstrcmp(argv[i], "-help") == 0 ) {
            printUsage();
            return 0;
        } else if ( argv[i][0] == '-' ) {
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
        if ( argv[i][0] == '-' )
            continue;

        QFile f( argv[i] );
        if ( !f.open(QIODevice::ReadOnly) ) {
#if defined(_MSC_VER) && _MSC_VER >= 1400
			char buf[100];
			strerror_s(buf, sizeof(buf), errno);
			fprintf( stderr,
                     "lrelease error: Cannot open file '%s': %s\n", argv[i],
                     buf );
#else
            fprintf( stderr,
                     "lrelease error: Cannot open file '%s': %s\n", argv[i],
                     strerror(errno) );
#endif
            return 1;
        }

        QTextStream t( &f );
        QString fullText = t.readAll();
        f.close();

        if ( fullText.contains(QString("<!DOCTYPE TS>"))) {
            if ( outputFile.isEmpty() ) {
                releaseTsFile( argv[i], verbose, ignoreUnfinished,
                               trimmed );
            } else {
                loadTsFile( tor, argv[i], verbose );
            }
        } else {
            QString oldDir = QDir::currentPath();
            QDir::setCurrent( QFileInfo(argv[i]).path() );

            QMap<QString, QString> tagMap;
            if (proFileTagMap( fullText, &tagMap )) {
                QMap<QString, QString>::Iterator it;

                for ( it = tagMap.begin(); it != tagMap.end(); ++it ) {
                    QStringList toks = it.value().split(' ');
                    QStringList::Iterator t;

                    for ( t = toks.begin(); t != toks.end(); ++t ) {
                        if ( it.key() == QString("TRANSLATIONS") ) {
                            metTranslations = true;
                            releaseTsFile( *t, verbose, ignoreUnfinished,
                                           trimmed );
                        }
                    }
                }
                if ( !metTranslations )
                    fprintf( stderr,
                             "lrelease warning: Met no 'TRANSLATIONS' entry in"
                             " project file '%s'\n",
                             argv[i] );
                QDir::setCurrent( oldDir );
            } else {
                fprintf( stderr, "error: lrelease encountered project file functionality that is currently not supported.\n"
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
