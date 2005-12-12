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

#include "metatranslator.h"
#include "proparser.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include <errno.h>
#include <string.h>

// defined in fetchtr.cpp
extern void fetchtr_cpp( const char *fileName, MetaTranslator *tor,
                         const char *defaultContext, bool mustExist, const QByteArray &codecForSource );
extern void fetchtr_ui( const char *fileName, MetaTranslator *tor,
                        const char *defaultContext, bool mustExist );

// defined in merge.cpp
extern void merge( const MetaTranslator *tor, const MetaTranslator *virginTor, MetaTranslator *out,
                   bool verbose );

typedef QList<MetaTranslatorMessage> TML;

static void printUsage()
{
    fprintf( stderr, "Usage:\n"
        "    lupdate [options] [project-file]\n"
             "    lupdate [options] [source-file|path]... -ts ts-files\n"
             "Options:\n"
             "    -help  Display this information and exit.\n"
             "    -noobsolete\n"
             "           Drop all obsolete strings.\n"
             "    -extensions <ext>[,<ext>]...\n"
             "           Process files with the given extensions only.\n"
             "           The extension list must be separated with commas, not with whitespace.\n"
             "           Default: 'cpp,h,ui'.\n"
             "    -verbose\n"
             "           Explain what is being done.\n"
             "    -version\n"
             "           Display the version of lupdate and exit.\n");
}

static void updateTsFiles( const MetaTranslator& fetchedTor,
                           const QStringList& tsFileNames, const QString& codecForTr,
						   bool noObsolete, bool verbose )
{
    QStringList::ConstIterator t = tsFileNames.begin();
    while ( t != tsFileNames.end() ) {
        MetaTranslator tor;
        MetaTranslator out;
        tor.load( *t );
        if ( !codecForTr.isEmpty() )
            tor.setCodec( codecForTr.toLatin1() );
        if ( verbose )
            fprintf( stderr, "Updating '%s'...\n", (*t).toLatin1().data() );
        merge( &tor, &fetchedTor, &out, verbose );
        if ( noObsolete )
            out.stripObsoleteMessages();
        out.stripEmptyContexts();
	    if ( !out.save(*t, verbose) ) {
#if defined(_MSC_VER) && _MSC_VER >= 1400
	        char buf[100];
	        strerror_s(buf, sizeof(buf), errno);
	        fprintf( stderr, "lupdate error: Cannot save '%s': %s\n",
                     (*t).toLatin1().constData(), buf );
#else
            fprintf( stderr, "lupdate error: Cannot save '%s': %s\n",
                     (*t).toLatin1().constData(), strerror(errno) );
#endif
	    }
        ++t;
    }
}

void recursiveFileInfoList(const QDir &dir, const QStringList &nameFilters, QDir::Filters filter, bool recursive, QFileInfoList *fileinfolist)
{
    if (recursive) filter |= QDir::AllDirs;
    QFileInfoList entries = dir.entryInfoList(nameFilters, filter);

    QFileInfoList::iterator it;
    for (it = entries.begin(); it != entries.end(); ++it) {
        QString fname = it->fileName();
        if (fname != QLatin1String(".") && fname != QLatin1String("..")) {
            if (it->isDir()) {
                recursiveFileInfoList(QDir(it->absoluteFilePath()), nameFilters, filter, recursive, fileinfolist);
            }else {
                fileinfolist->append(*it);
            }
        }
    }
}

QString relativePath(const QString &absolutePath, const QString &currentPath)
{
    QString cur = currentPath.isNull() ? QDir::currentPath() : currentPath;
    QString ret = absolutePath;
    ret.remove(currentPath, Qt::CaseInsensitive);
    if (ret.startsWith(QChar('\\')) || ret.startsWith(QChar('/'))) ret.remove(0,1);
    return ret;
}

int main( int argc, char **argv )
{
    QString defaultContext = "@default";
    MetaTranslator fetchedTor;
    QByteArray codecForTr;
	QByteArray codecForSource;
    QStringList tsFileNames;

    bool verbose = false;
    bool noObsolete = false;
    bool metSomething = false;
    int numFiles = 0;
    bool standardSyntax = true;
    bool metTsFlag = false;

    QString extensions = QLatin1String("cpp,h,ui");
    QStringList extensionsNameFilters;
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
        } else if ( qstrcmp(argv[i], "-extensions") == 0 ) {
            ++i;
            if (i == argc) {
                fprintf( stderr, "The -extensions option should be followed by an extension list.");
                return 1;
            }
            extensions = QString(QLatin1String(argv[i]));            
            continue;
        }

        numFiles++;

        QString fullText;

        if ( standardSyntax && !metTsFlag ) {
            QFile f( argv[i] );
            if ( !f.open(QIODevice::ReadOnly) ) {
#if defined(_MSC_VER) && _MSC_VER >= 1400
				char buf[100];
				strerror_s(buf, sizeof(buf), errno);
				fprintf( stderr, "lupdate error: Cannot open file '%s': %s\n",
                         argv[i], buf );
#else
                fprintf( stderr, "lupdate error: Cannot open file '%s': %s\n",
                         argv[i], strerror(errno) );
#endif
                return 1;
            }

            QTextStream t( &f );
            fullText = t.readAll();
            f.close();
        }

        QString oldDir = QDir::currentPath();

        if ( standardSyntax ) {
            QDir::setCurrent( QFileInfo(argv[i]).path() );
            fetchedTor = MetaTranslator();
            codecForTr.clear();
			codecForSource.clear();
            tsFileNames.clear();

            QMap<QString, QString> tagMap;
            if (proFileTagMap( fullText, &tagMap )) {
                QMap<QString, QString>::Iterator it;

                for ( it = tagMap.begin(); it != tagMap.end(); ++it ) {
                    QStringList toks = tokenizeFileNames(it.value());
                    QStringList::Iterator t;

                    for ( t = toks.begin(); t != toks.end(); ++t ) {
                        if ( it.key() == "HEADERS" || it.key() == "SOURCES" ) {
                            fetchtr_cpp( (*t).toAscii(), &fetchedTor, defaultContext.toAscii(), true, codecForSource );
                            metSomething = true;
                        } else if ( it.key() == "INTERFACES" ||
                                    it.key() == "FORMS" ) {
                            fetchtr_ui( (*t).toAscii(), &fetchedTor, defaultContext.toAscii(), true );
                            fetchtr_cpp( (*t).toAscii() + ".h", &fetchedTor, defaultContext.toAscii(), false, codecForSource );
                            metSomething = true;
                        } else if ( it.key() == "TRANSLATIONS" ) {
                            tsFileNames.append( *t );
                            metSomething = true;
                        } else if ( it.key() == "CODEC" ||
                                    it.key() == "DEFAULTCODEC" ||
                                    it.key() == "CODECFORTR" ) {
                            codecForTr = (*t).toLatin1();
                        } else if ( it.key() == "CODECFORSRC" ) {
                            codecForSource = (*t).toLatin1();
                        }
                    }
                }

                updateTsFiles( fetchedTor, tsFileNames, codecForTr, noObsolete, verbose );

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
                if (fi.isDir()) {
                    if ( verbose ) fprintf(stderr, "Scanning directory '%s'...\n", argv[i]);
                    QDir dir = QDir(fi.filePath());
                    if (extensionsNameFilters.isEmpty()) {
                        extensions = extensions.trimmed();
                        // Remove the potential dot in front of each extension
                        if (extensions.startsWith('.'))
                            extensions.remove(0,1);
                        extensions.replace(",.", ",");

                        extensions.insert(0, QLatin1String("*."));
                        extensions.replace(',', QLatin1String(",*."));
                        extensionsNameFilters = extensions.split(',');
                    }
                    QDir::Filters filters = QDir::Files | QDir::Filter::NoSymLinks;
                    QFileInfoList fileinfolist;
                    recursiveFileInfoList(dir, extensionsNameFilters, filters, true, &fileinfolist);
                    QFileInfoList::iterator ii;
                    QByteArray fn;
                    for (ii = fileinfolist.begin(); ii != fileinfolist.end(); ++ii) {
                        fn = relativePath(ii->filePath(), oldDir).toLatin1();
#ifdef LINGUIST_DEBUG
                        fprintf(stderr, "%s\n", fn.data());
#endif
                        if (fn.toLower().endsWith(".ui") ) {
                            fetchtr_ui( fn, &fetchedTor, defaultContext.toAscii(), true );
                        } else {
                            fetchtr_cpp( fn, &fetchedTor, defaultContext.toAscii(), true, codecForSource );
                        }
                    }
                }else{
                    if ( QString(argv[i]).toLower().endsWith(".ui") ) {
                        fetchtr_ui( fi.fileName().toAscii(), &fetchedTor, defaultContext.toAscii(), true );
                        fetchtr_cpp( fi.fileName().toAscii() + ".h", &fetchedTor,
                                     defaultContext.toAscii(), false, codecForSource );
                    } else {
                        fetchtr_cpp( fi.fileName().toAscii(), &fetchedTor, defaultContext.toAscii(), true, codecForSource );
                    }
                }
            }
        }
        QDir::setCurrent( oldDir );
    }

    if ( !standardSyntax )
        updateTsFiles( fetchedTor, tsFileNames, codecForTr, noObsolete, verbose );

    if ( numFiles == 0 ) {
        printUsage();
        return 1;
    }
    return 0;
}
