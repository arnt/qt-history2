#include "lupdate.h"

#include <proparser.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QCoreApplication>

#include <errno.h>
#include <string.h>
#include <QtCore/QDebug>

typedef QList<MetaTranslatorMessage> TML;

void lupdateApplication::recursiveFileInfoList(const QDir &dir, const QStringList &nameFilters, 
                           QDir::Filters filter, bool recursive, QFileInfoList *fileinfolist)
{
    if (recursive)
        filter |= QDir::AllDirs;
    QFileInfoList entries = dir.entryInfoList(nameFilters, filter);

    QFileInfoList::iterator it;
    for (it = entries.begin(); it != entries.end(); ++it) {
        QString fname = it->fileName();
        if (fname != QLatin1String(".") && fname != QLatin1String("..")) {
            if (it->isDir()) {
                recursiveFileInfoList(QDir(it->absoluteFilePath()), nameFilters, filter, recursive, fileinfolist);
            } else {
                fileinfolist->append(*it);
            }
        }
    }
}

void lupdateApplication::printUsage()
{
    Console::out( tr("Usage:\n"
             "    lupdate [options] [project-file]\n"
             "    lupdate [options] [source-file|path]... -ts ts-files\n"
             "Options:\n"
             "    -help  Display this information and exit.\n"
             "    -noobsolete\n"
             "           Drop all obsolete strings.\n"
             "    -extensions <ext>[,<ext>]...\n"
             "           Process files with the given extensions only.\n"
             "           The extension list must be separated with commas, not with whitespace.\n"
             "           Default: '%1'.\n"
             "    -silent\n"
             "           Don't explain what is being done.\n"
             "    -version\n"
             "           Display the version of lupdate and exit.\n").arg(m_defaultExtensions));
}

void lupdateApplication::updateTsFiles( const MetaTranslator& fetchedTor,
                           const QStringList& tsFileNames, const QString& codecForTr,
                           bool noObsolete, bool verbose )
{
    QStringList::ConstIterator t = tsFileNames.begin();
    QDir dir;
    while ( t != tsFileNames.end() ) {
        QString fn = dir.relativeFilePath(*t);
        MetaTranslator tor;
        MetaTranslator out;
        tor.load( *t );
        if ( !codecForTr.isEmpty() ) {
            out.setCodec( codecForTr.toLatin1() );
            tor.setCodec( codecForTr.toLatin1() );
        }
        if ( verbose )
            Console::out(tr("Updating '%1'...\n").arg(fn));
 
        merge( &tor, &fetchedTor, &out, verbose, noObsolete );
        if ( noObsolete )
            out.stripObsoleteMessages();
        out.stripEmptyContexts();
        
            if ( !out.save(*t) ) {
#if defined(_MSC_VER) && _MSC_VER >= 1400
                char buf[100];
                strerror_s(buf, sizeof(buf), errno);
                qWarning("lupdate error: Cannot save '%s': %s\n",
                     fn.toLatin1().constData(), buf );
#else
            qWarning( "lupdate error: Cannot save '%s': %s\n",
                     fn.toLatin1().constData(), strerror(errno) );
#endif
            }
        ++t;
    }
}


int lupdateApplication::start()
{
    int argc = arguments().count();
    QStringList argv = arguments();
    QString defaultContext = QLatin1String("@default");
    MetaTranslator fetchedTor;
    QByteArray codecForTr;
    QByteArray codecForSource;
    QStringList tsFileNames;
    QStringList proFiles;
    QStringList sourceFiles;

    bool verbose = true; // verbose is on by default starting with Qt 4.2
    bool noObsolete = false;
    int numFiles = 0;
    bool standardSyntax = true;
    bool metTsFlag = false;

    QString extensions = QLatin1String(m_defaultExtensions);
    QStringList extensionsNameFilters;
    int i;

    for ( i = 1; i < argc; i++ ) {
        if ( argv.at(i) == QLatin1String("-ts") )
            standardSyntax = false;
    }

    QString oldDir = QDir::currentPath();

    for ( i = 1; i < argc; i++ ) {
        QString arg = argv.at(i);
        QByteArray barg = arg.toLocal8Bit();
        if ( barg == "-help") {
            printUsage();
            return 0;
        } else if ( barg == "-noobsolete" ) {
            noObsolete = true;
            continue;
        } else if ( barg == "-silent") {
            verbose = false;
            continue;
        } else if ( barg == "-verbose") {
            verbose = true;
            continue;
        } else if ( barg == "-version") {
            Console::out(tr("lupdate version %1\n").arg(QT_VERSION_STR) );
            return 0;
        } else if ( barg == "-ts") {
            metTsFlag = true;
            continue;
        } else if ( barg == "-extensions") {
            ++i;
            if (i == argc) {
                qWarning("The -extensions option should be followed by an extension list.");
                return 1;
            }
            extensions = arg;
            continue;
        }

        numFiles++;
        
        QString fullText;

        if ( standardSyntax && !metTsFlag ) {
            QFile f( arg );
            if ( !f.open(QIODevice::ReadOnly) ) {
#if defined(_MSC_VER) && _MSC_VER >= 1400
				char buf[100];
				strerror_s(buf, sizeof(buf), errno);
				qWarning("lupdate error: Cannot open file '%s': %s\n",
                         barg.constData(), buf );
#else
                qWarning("lupdate error: Cannot open file '%s': %s\n",
                         barg.constData(), strerror(errno) );
#endif
                return 1;
            }
            f.close();
        }

        codecForTr.clear();
		codecForSource.clear();

        if (metTsFlag) {
            if ( arg.endsWith(QLatin1String(".ts"), Qt::CaseInsensitive) 
                || arg.endsWith(QLatin1String(".xlf"), Qt::CaseInsensitive)) {
                QFileInfo fi( arg );
                if ( !fi.exists() || fi.isWritable() ) {
                    tsFileNames.append( QFileInfo(arg).absoluteFilePath() );
                } else {
                    qWarning("lupdate warning: For some reason, I cannot save '%s'\n",
                             barg.constData() );
                }
            } else {
                qWarning("lupdate error: File '%s' lacks .ts or .xlf extension\n",
                         barg.constData() );
            }
        } else if (arg.endsWith(QLatin1String(".pro"), Qt::CaseInsensitive)) {
            proFiles << arg;
        } else {
            QFileInfo fi(arg);
            if (fi.isDir()) {
                if ( verbose ) 
                    Console::out(tr("Scanning directory '%1'...\n").arg(arg));
                QDir dir = QDir(fi.filePath());
                if (extensionsNameFilters.isEmpty()) {
                    extensions = extensions.trimmed();
                    // Remove the potential dot in front of each extension
                    if (extensions.startsWith(QLatin1Char('.')))
                        extensions.remove(0,1);
                    extensions.replace(QLatin1String(",."), QLatin1String(","));

                    extensions.insert(0, QLatin1String("*."));
                    extensions.replace(QLatin1Char(','), QLatin1String(",*."));
                    extensionsNameFilters = extensions.split(QLatin1Char(','));
                }
                QDir::Filters filters = QDir::Files | QDir::NoSymLinks;
                QFileInfoList fileinfolist;
                recursiveFileInfoList(dir, extensionsNameFilters, filters, true, &fileinfolist);
                QFileInfoList::iterator ii;
                QString fn;
                QDir baseDir(oldDir);
                for (ii = fileinfolist.begin(); ii != fileinfolist.end(); ++ii) {
                    // Make sure the path separator is stored with '/' in the ts file
                    fn = ii->canonicalFilePath().replace(QLatin1Char('\\'),QLatin1Char('/'));
#ifdef LINGUIST_DEBUG
                    qDebug() << fn;
#endif
                    sourceFiles << fn;
                }
            }else{
                sourceFiles << fi.canonicalFilePath().replace(QLatin1Char('\\'),QLatin1Char('/'));
            }            
        }
    }   //for

    
    if ( proFiles.count() > 0 ) {
        proFiles = getListOfProfiles(proFiles, verbose);
    }
    bool firstPass = true;
    for (int pi = 0; firstPass || pi < proFiles.count(); ++pi) {
        QStringList tsFiles = tsFileNames;
        if (proFiles.count() > 0) {
            QString pf = proFiles.at(pi);
            QDir::setCurrent( QFileInfo(pf).path() );
            QMap<QByteArray, QStringList> variables;

            if(!evaluateProFile(QFileInfo(pf).fileName(), verbose, &variables))
                return 2;

            sourceFiles = variables.value("SOURCES");

            QStringList tmp = variables.value("CODECFORTR");
            if (!tmp.isEmpty()) {
                codecForTr = tmp.first().toAscii();
                fetchedTor.setCodecForTr(codecForTr.constData());
            }
            tmp = variables.value("CODECFORSRC");
            if (!tmp.isEmpty()) {
                codecForSource = tmp.first().toAscii();
            }

            tsFiles += variables.value("TRANSLATIONS");
        }

        for (QStringList::iterator it = sourceFiles.begin(); it != sourceFiles.end(); ++it) {
#ifdef LINGUIST_DEBUG
            qDebug() << "  " << (*it);
#endif
	    if ( (*it).endsWith(QLatin1String(".java"), Qt::CaseInsensitive) ) {
	        fetchtr_java( (*it).toAscii(), &fetchedTor, defaultContext.toAscii(), true, codecForSource );
	    }
            else if ( (*it).endsWith(QLatin1String(".ui"), Qt::CaseInsensitive) ) {
#ifdef LINGUIST_DEBUG
                qDebug() << "  " << (*it) + ".h";
#endif
                fetchtr_ui( (*it).toAscii(), &fetchedTor, defaultContext.toAscii(), true );
                fetchtr_cpp( QString((*it) + QLatin1String(".h")).toAscii(), &fetchedTor,
                             defaultContext.toAscii(), false, codecForSource );
            }else{
                fetchtr_cpp( (*it).toAscii(), &fetchedTor, defaultContext.toAscii(), true, codecForSource );
            }
        }

        removeDuplicates(&tsFiles, false);
        
        QDir::setCurrent( oldDir );
        if ( tsFiles.count() > 0) {
            updateTsFiles( fetchedTor, tsFiles, QString::fromLatin1(codecForTr.constData()), noObsolete, verbose );
        }
        firstPass = false;
    }
    QDir::setCurrent( oldDir );

    if ( numFiles == 0 ) {
        printUsage();
        return 1;
    }
    return 0;

}
