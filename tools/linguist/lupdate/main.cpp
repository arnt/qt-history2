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

#include <metatranslator.h>
#include <proparser.h>

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextstream.h>

#include <errno.h>
#include <string.h>

// defined in fetchtr.cpp
extern void fetchtr_cpp( const char *fileName, MetaTranslator *tor,
                         const char *defaultContext, bool mustExist );
extern void fetchtr_ui( const char *fileName, MetaTranslator *tor,
                        const char *defaultContext, bool mustExist );

// defined in merge.cpp
extern void merge( MetaTranslator *tor, const MetaTranslator *virginTor,
                   bool verbose );

typedef QList<MetaTranslatorMessage> TML;

static void printUsage()
{
    fprintf( stderr, "Usage:\n"
             "    lupdate [options] project-file\n"
             "    lupdate [options] source-files -ts ts-files\n"
             "Options:\n"
             "    -help  Display this information and exit\n"
             "    -noobsolete\n"
             "           Drop all obsolete strings\n"
             "    -verbose\n"
             "           Explain what is being done\n"
             "    -version\n"
             "           Display the version of lupdate and exit\n" );
}

static void updateTsFiles( const MetaTranslator& fetchedTor,
                           const QStringList& tsFileNames, const QString& codec,
                           bool noObsolete, bool verbose )
{
    QStringList::ConstIterator t = tsFileNames.begin();
    while ( t != tsFileNames.end() ) {
        MetaTranslator tor;
        tor.load( *t );
        if ( !codec.isEmpty() )
            tor.setCodec( codec.toLatin1() );
        if ( verbose )
            fprintf( stderr, "Updating '%s'...\n", (*t).toLatin1().data() );
        merge( &tor, &fetchedTor, verbose );
        if ( noObsolete )
            tor.stripObsoleteMessages();
        tor.stripEmptyContexts();
        if ( !tor.save(*t) )
            fprintf( stderr, "lupdate error: Cannot save '%s': %s\n",
                     (*t).toLatin1().constData(), strerror(errno) );
        ++t;
    }
}

int main( int argc, char **argv )
{
    QString defaultContext = "@default";
    MetaTranslator fetchedTor;
    QByteArray codec;
    QStringList tsFileNames;

    bool verbose = false;
    bool noObsolete = false;
    bool metSomething = false;
    int numFiles = 0;
    bool standardSyntax = true;
    bool metTsFlag = false;

    int i;

    for ( i = 1; i < argc; i++ ) {
        if ( qstrcmp(argv[i], "-ts") == 0 )
            standardSyntax = false;
    }

    for ( i = 1; i < argc; i++ ) {
        if ( qstrcmp(argv[i], "-help") == 0 ) {
            printUsage();
            return 0;
        } else if ( qstrcmp(argv[i], "-noobsolete") == 0 ) {
            noObsolete = true;
            continue;
        } else if ( qstrcmp(argv[i], "-verbose") == 0 ) {
            verbose = true;
            continue;
        } else if ( qstrcmp(argv[i], "-version") == 0 ) {
            fprintf( stderr, "lupdate version %s\n", QT_VERSION_STR );
            return 0;
        } else if ( qstrcmp(argv[i], "-ts") == 0 ) {
            metTsFlag = true;
            continue;
        }

        numFiles++;

        QString fullText;

        if ( !metTsFlag ) {
            QFile f( argv[i] );
            if ( !f.open(QIODevice::ReadOnly) ) {
                fprintf( stderr, "lupdate error: Cannot open file '%s': %s\n",
                         argv[i], strerror(errno) );
                return 1;
            }

            QTextStream t( &f );
            fullText = t.read();
            f.close();
        }

        QString oldDir = QDir::currentPath();
        QDir::setCurrent( QFileInfo(argv[i]).path() );

        if ( standardSyntax ) {
            fetchedTor = MetaTranslator();
            codec.truncate( 0 );
            tsFileNames.clear();

            QMap<QString, QString> tagMap = proFileTagMap( fullText );
            QMap<QString, QString>::Iterator it;

            for ( it = tagMap.begin(); it != tagMap.end(); ++it ) {
                QStringList toks = it.value().split(' ');
                QStringList::Iterator t;

                for ( t = toks.begin(); t != toks.end(); ++t ) {
                    if ( it.key() == "HEADERS" || it.key() == "SOURCES" ) {
                        fetchtr_cpp( (*t).toAscii(), &fetchedTor, defaultContext.toAscii(), true );
                        metSomething = true;
                    } else if ( it.key() == "INTERFACES" ||
                                it.key() == "FORMS" ) {
                        fetchtr_ui( (*t).toAscii(), &fetchedTor, defaultContext.toAscii(), true );
                        fetchtr_cpp( (*t).toAscii() + ".h", &fetchedTor, defaultContext.toAscii(), false );
                        metSomething = true;
                    } else if ( it.key() == "TRANSLATIONS" ) {
                        tsFileNames.append( *t );
                        metSomething = true;
                    } else if ( it.key() == "CODEC" ||
                                it.key() == "DEFAULTCODEC" ) {
                        codec = (*t).toLatin1();
                    }
                }
            }

            updateTsFiles( fetchedTor, tsFileNames, codec, noObsolete,
                           verbose );

            if ( !metSomething ) {
                fprintf( stderr,
                         "lupdate warning: File '%s' does not look like a"
                         " project file\n",
                         argv[i] );
            } else if ( tsFileNames.isEmpty() ) {
                fprintf( stderr,
                         "lupdate warning: Met no 'TRANSLATIONS' entry in"
                         " project file '%s'\n",
                         argv[i] );
            }
        } else {
            if ( metTsFlag ) {
                if ( QString(argv[i]).toLower().endsWith(".ts") ) {
                    QFileInfo fi( argv[i] );
                    if ( !fi.exists() || fi.isWritable() ) {
                        tsFileNames.append( argv[i] );
                    } else {
                        fprintf( stderr,
                                 "lupdate warning: For some reason, I cannot"
                                 " save '%s'\n",
                                 argv[i] );
                    }
                } else {
                    fprintf( stderr,
                             "lupdate error: File '%s' lacks .ts extension\n",
                             argv[i] );
                }
            } else {
                QFileInfo fi(argv[i]);
                if ( QString(argv[i]).toLower().endsWith(".ui") ) {
                    fetchtr_ui( fi.fileName().toAscii(), &fetchedTor, defaultContext.toAscii(), true );
                    fetchtr_cpp( fi.fileName().toAscii() + ".h", &fetchedTor,
                                 defaultContext.toAscii(), false );
                } else {
                    fetchtr_cpp( fi.fileName().toAscii(), &fetchedTor, defaultContext.toAscii(), true );
                }
            }
        }
        QDir::setCurrent( oldDir );
    }

    if ( !standardSyntax )
        updateTsFiles( fetchedTor, tsFileNames, codec, noObsolete, verbose );

    if ( numFiles == 0 ) {
        printUsage();
        return 1;
    }
    return 0;
}
