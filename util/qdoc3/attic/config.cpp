/*
  config.cpp
*/

#include <qdir.h>
#include <qfile.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextstream.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "location.h"
#include "messages.h"

Config *config;

static bool isCSym( QChar ch )
{
    return ch.isLetterOrNumber() || ch == QChar( '_' );
}

static QString eval( const QString& str )
{
    QString t = str;
    int left = -1;
    while ( (left = t.findRev(QChar('$'), left)) != -1 ) {
	int right = left + 1;
	while ( right < (int) t.length() && isCSym(t[right]) )
	    right++;

	QString key = t.mid( left + 1, right - left - 1 );
	char *val = getenv( key.latin1() );
	if ( val != 0 )
	    t.replace( left, right - left, QString(val) );

	if ( --left == -1 )
	    break;
    }
    return t;
}

static QString singleton( const QString& key, const QStringList& val )
{
    if ( val.count() != 1 ) {
	message( 1, "Entry '%s' should contain exactly one value (found %d)",
		 key.latin1(), val.count() );
	if ( val.isEmpty() )
	    return QString( "" );
	else
	    return val.first();
    }
    return val.first();
}

static bool isYes( const QString& val )
{
    return val == QString( "yes" ) || val == QString( "true" );
}

static bool isYes( const QString& key, const QStringList& val )
{
    if ( val.count() != 1 ||
	 !QRegExp(QString("yes|no|true|false")).exactMatch(val.first()) ) {
	message( 1, "Entry '%s' in configuration file should be 'yes' or 'no'",
		 key.latin1() );
	return FALSE;
    }
    return val.first() == QString( "yes" ) || val.first() == QString( "true" );
}

static const char *toYN( bool yes )
{
    return yes ? "yes" : "no";
}

static int numParams( const QString& t )
{
    int n = 0;

    int k = 0;
    while ( (k = t.find(QRegExp(QString("%[1-9]")), k)) != -1 ) {
	int d = t[k + 1].digitValue();
	if ( d > n )
	    n = d;
	k += 2;
    }
    return n;
}

static QString replaceArgs( const QString& s, const QStringList& args )
{
    QString t = s;
    int d;

    int k = 0;
    while ( (k = t.find(QChar('%'), k)) != -1 ) {
	if ( k + 1 < (int) t.length() ) {
	    if ( t[k + 1] == QChar('%') ) {
		t.replace( k, 2, QChar('%') );
		k++;
	    } else if ( (d = t[k + 1].digitValue()) > 0 ) {
		t.replace( k, 2, args[d - 1] );
		k += args[d - 1].length();
	    } else {
		k++;
	    }
	} else {
	    k++;
	}
    }
    return t;
}

static void setPattern( QRegExp *rx, const QString& pattern, bool plus )
{
    static QRegExp separators( QString("[ \t\n]*[ \t\n,;][ \t\n]*") );

    QString t = pattern.stripWhiteSpace();
    t.replace( separators, QChar('|') );

    if ( plus && !rx->pattern().isEmpty() && rx->isValid() )
	rx->setPattern( rx->pattern() + QChar('|') + t );
    else
	rx->setPattern( t );
}

static int getBigEndianInt( const char *t )
{
    uint n = 0;
    for ( int i = 0; i < 4; i++ )
	n = (long) ( (n << 8) | (uchar) t[i] );
    return (int) n;
}

static bool getImageSize( const char *fileName, int *width, int *height )
{
    char magic[8];
    char chunkHeader[8];
    char ihdr[13];

    FILE *in = fopen( fileName, "rb" );
    if ( in == 0 )
	return FALSE;

    bool result = FALSE;
    if ( fread(magic, 1, 8, in) == 8 &&
	 memcmp(magic, "\211PNG\r\n\032\n", 8) == 0 ) {
	while ( fread(chunkHeader, 1, 8, in) == 8 ) {
	    if ( memcmp(chunkHeader + 4, "IHDR", 4) == 0 ) {
		if ( fread(ihdr, 1, 13, in) != 13 )
		    break;
		result = TRUE;
		*width = getBigEndianInt( ihdr );
		*height = getBigEndianInt( ihdr + 4 );
		break;
	    } else {
		int len = getBigEndianInt( chunkHeader );
		if ( fseek(in, (long) (len + 4), SEEK_CUR) != 0 )
		    break;
	    }
	}
    }
    fclose( in );
    return result;
}

Config::Config( int argc, char **argv )
    : maxSim( 10 ), maxAll( 100 ), wlevel( 3 ), bas( "" ), prod( "" ), co( "" ),
      vers( "" ), verssym( "" ), posth( "" ), foot( "" ), addr( "" ),
      styl( "" ), falsesym( QChar('0') ), internal( FALSE ), autoh( TRUE ),
      super( FALSE ), lin( FALSE ), frend( FALSE ), dotHtml( ".html" ),
      membersDotHtml( "-members.html" ), dotPng( ".png" )
{
    QString confFilePath( "qdoc.conf" );
    int i;

    i = 1;
    while ( i < argc ) {
	QString opt( argv[i++] );

	if ( opt == QString("--version") || opt == QString("-v") ) {
	    argv[i - 1][0] = '\0';
	    showVersion();
	} else if ( opt == QString("--") ) {
	    argv[i - 1][0] = '\0';
	    if ( i < argc )
		confFilePath = QString( argv[i] );
	    while ( i < argc )
		argv[i++][0] = '\0';
	} else if ( !opt.startsWith(QChar('-')) ) {
	    confFilePath = opt;
	    argv[i - 1][0] = '\0';
	    break;
	}
    }

    QFile f( confFilePath );
    if ( !f.open(IO_ReadOnly) ) {
	message( 0, "Cannot open configuration file '%s'",
		 confFilePath.latin1() );
	exit( EXIT_FAILURE );
    }

    QTextStream t( &f );
    yyIn = t.read();
    f.close();
    yyPos = 0;

    while ( yyPos < (int) yyIn.length() ) {
	QString key;
	QStringList val;
	if ( !matchLine(&key, &val) )
	    break;

	if ( key.contains(QChar('.')) == 1 ) {
	    QString t = val.join( QChar(' ') );
	    aliasMap.insert( key + QString::number(numParams(t)), t );
	} else {	
	    if ( key == QString("ADDRESS") ) {
		addr = val.join( QChar(' ') );
	    } else if ( key == QString("AUTOHREFS") ) {
		autoh = isYes( singleton(key, val) );
	    } else if ( key == QString("BASE") ) {
		bas = val.join( QChar(' ') );
	    } else if ( key == QString("BOOKDIRS") ) {
		bookdirs = val;
	    } else if ( key == QString("COMPANY") ) {
		co = val.join( QChar(' ') );
	    } else if ( key == QString("DEFINE") ) {
		defsym.setPattern( val.join(QChar('|')) );
	    } else if ( key == QString("DOCDIRS") ) {
		docdirs = val;
	    } else if ( key == QString("EXAMPLEDIRS") ) {
		exampledirs = val;
	    } else if ( key == QString("FALSE") ) {
		falsesym.setPattern( val.join(QChar('|')) );
	    } else if ( key == QString("FOOTER") ) {
		foot = val.join( QChar(' ') );
	    } else if ( key == QString("IMAGEDIRS") ) {
		imagedirs = val;
	    } else if ( key == QString("INCLUDEDIRS") ) {
		includedirs = val;
	    } else if ( key == QString("INTERNAL") ) {
		internal = isYes( key, val );
	    } else if ( key == QString("MAXSIMILAR") ) {
		maxSim = singleton( key, val ).toInt();
	    } else if ( key == QString("MAXWARNINGS") ) {
		maxAll = singleton( key, val ).toInt();
	    } else if ( key == QString("ONLY") ) {
		onlyfn.setPattern( val.join(QChar('|')) );
	    } else if ( key == QString("OUTPUTDIR") ) {
		outputdir = singleton( key, val );
	    } else if ( key == QString("POSTHEADER") ) {
		posth = val.join( QChar(' ') );
	    } else if ( key == QString("PRODUCT") ) {
		prod = val.join( QChar(' ') );
	    } else if ( key == QString("SOURCEDIRS") ) {
		sourcedirs = val;
	    } else if ( key == QString("STYLE") ) {
		styl = val.join( QChar(' ') );
	    } else if ( key == QString("SUPERVISOR") ) {
		super = isYes( key, val );
	    } else if ( key == QString("VERSIONSTR") ) {
		vers = val.join( QChar(' ') );
	    } else if ( key == QString("VERSIONSYM") ) {
		verssym = singleton( key, val );
	    } else if ( key == QString("WARNINGLEVEL") ) {
		wlevel = singleton( key, val ).toInt();
		if ( wlevel < 0 )
		    wlevel = 0;
	    } else {
		message( 1, "Unknown entry '%s' in configuration file",
			 key.latin1() );
	    }
	}
    }

    i = 1;
    while ( i < argc ) {
	QString opt( argv[i++] );
	QString val;
	bool plus = FALSE;

	if ( opt.startsWith(QString("--")) ) {
	    int k = opt.find( QChar('=') );
	    if ( k == -1 ) {
		if ( i < argc && !QString(argv[i]).startsWith(QChar('-')) )
		    val = QString( argv[i++] );
		else
		    val = QString( "yes" );
	    } else {
		val = opt.mid( k + 1 );
		opt.truncate( k );
		if ( opt.right(1) == QChar('+') ) {
		    plus = TRUE;
		    opt.truncate( k - 1 );
		}
	    }

	    if ( opt == QString("--auto-hrefs") ) {
		autoh = isYes( val );
	    } else if ( opt == QString("--base") ) {
		bas = val;
	    } else if ( opt == QString("--define") ) {
		setPattern( &defsym, val, plus );
	    } else if ( opt == QString("--false") ) {
		setPattern( &falsesym, val, plus );
	    } else if ( opt == QString("--friendly") ) {
		frend = isYes( val );
	    } else if ( opt == QString("--help") ) {
		showHelp();
	    } else if ( opt == QString("--help-short") ) {
		showHelpShort();
	    } else if ( opt == QString("--internal") ) {
		internal = isYes( val );
	    } else if ( opt == QString("--lint") ) {
		lin = isYes( val );
	    } else if ( opt == QString("--max-similar") ) {
		if ( !plus )
		    maxSim = 0;
		maxSim += val.toInt();
	    } else if ( opt == QString("--max-warnings") ) {
		if ( !plus )
		    maxAll = 0;
		maxAll += val.toInt();
	    } else if ( opt == QString("--only") ) {
		setPattern( &onlyfn, val, plus );
	    } else if ( opt == QString("--output-dir") ) {
		outputdir = val;
	    } else if ( opt == QString("--supervisor") ) {
		super = isYes( val );
	    } else if ( opt == QString("--warning-level") ) {
		if ( !plus )
		    wlevel = 0;
		wlevel += val.toInt();
	    } else {
		message( 0, "Unknown command-line option '%s'", opt.latin1() );
		showHelp();
	    }
	} else if ( opt.startsWith(QChar('-')) ) {
	    if ( opt == QString("-a") ) {
		autoh = TRUE;
	    } else if ( opt == QString("-A") ) {
		autoh = FALSE;
	    } else if ( opt.startsWith(QString("-D")) ) {
		setPattern( &defsym, opt.mid(2), TRUE );
	    } else if ( opt.startsWith(QString("-F")) ) {
		setPattern( &falsesym, opt.mid(2), TRUE );
	    } else if ( opt == QString("-h") ) {
		showHelp();
	    } else if ( opt == QString("-H") ) {
		showHelpShort();
	    } else if ( opt == QString("-i") ) {
		internal = TRUE;
	    } else if ( opt == QString("-I") ) {
		internal = FALSE;
	    } else if ( opt.startsWith(QString("-m")) ) {
		maxSim = opt.mid( 2 ).toInt();
	    } else if ( opt.startsWith(QString("-M")) ) {
		maxAll = opt.mid( 2 ).toInt();
	    } else if ( opt.startsWith(QString("-O")) ) {
		setPattern( &onlyfn, opt.mid(2), TRUE );
	    } else if ( opt == QString("-s") ) {
		super = TRUE;
	    } else if ( opt == QString("-S") ) {
		super = FALSE;
	    } else if ( QRegExp(QString("-W[0-4]")).exactMatch(opt) ) {
		wlevel = opt[2].unicode() - QChar( '0' ).unicode();
	    } else if ( opt == QString("-Wnone") ) {
		wlevel = -1;
	    } else if ( opt == QString("-Wall") ) {
		wlevel = 4;
		maxSim = 262144;
		maxAll = 262144;
	    } else {
		message( 0, "Unknown command-line option '%s'", opt.latin1() );
		showHelpShort();
	    }
	} else if ( !opt.isEmpty() ) {
	    message( 0, "Command-line argument '%s' ignored", opt.latin1() );
	    showHelp();
	}
    }

    onlyfn.setPattern( QString("(?:") + onlyfn.pattern() + QString(").*") );

    setMaxSimilarMessages( maxSim );
    setMaxMessages( maxAll );
    setWarningLevel( wlevel );

    yyIn = "";
}

void Config::setVersion( const QString& version )
{
    if ( vers.isEmpty() ) {
	vers = version;
	addr.replace( QRegExp(QString("\\\\version")), vers );
    }
}

QString Config::verbatimHref( const QString& sourceFileName ) const
{
    static QRegExp evil( QString("[./]") );

    QString t = sourceFileName;
    t.replace( evil, QChar('-') );
    t += dotHtml;
    return t;
}

QString Config::classRefHref( const QString& className ) const
{
    return className.lower() + dotHtml;
}

QString Config::classMembersHref( const QString& className ) const
{
    return className.lower() + membersDotHtml;
}

QString Config::classImageHref( const QString& className ) const
{
    return className.lower() + dotPng;
}

QString Config::defgroupHref( const QString& groupName ) const
{
    return groupName.lower() + dotHtml;
}

QString Config::findDepth( const QString& name,
			   const QStringList& dirList ) const
{
    QStringList::ConstIterator s;
    QString filePath;

    s = dirList.begin();
    while ( s != dirList.end() ) {
	QDir dir( *s );
	if ( dir.exists(name) )
	    return dir.filePath( name );
	++s;
    }
    return "";
}

QStringList Config::findAll( const QString& nameFilter,
			     const QStringList& dirList ) const
{
    QStringList result;

    QStringList::ConstIterator d = dirList.begin();
    while ( d != dirList.end() ) {
	QStringList fileNames;
	QStringList::Iterator fn;
	QDir dir( *d );

	dir.setNameFilter( nameFilter );
	dir.setSorting( QDir::Name );
	dir.setFilter( QDir::Files );
	fileNames = dir.entryList();
	fn = fileNames.begin();
	while ( fn != fileNames.end() ) {
	    result += dir.filePath( *fn );
	    ++fn;
	}

	dir.setNameFilter( QChar('*') );
	dir.setFilter( QDir::Dirs );
	fileNames = dir.entryList();
	fn = fileNames.begin();
	while ( fn != fileNames.end() ) {
	    if ( *fn != QChar('.') && *fn != QString("..") )
		result += findAll( nameFilter, dir.filePath(*fn) );
	    ++fn;
	}
	++d;
    }
    return result;
}

bool Config::isTrue( const QString& condition ) const
{
    return !falsesym.exactMatch( condition );
}


bool Config::isDef( const QString& symbol ) const
{
    return defsym.exactMatch( symbol );
}

bool Config::generateFile( const QString& fileName ) const
{
    return onlyfn.exactMatch( fileName );
}

bool Config::needImage( const Location& loc, const QString& fileName,
			int *width, int *height )
{
    QPair<int, int> size( 0, 0 );

    if ( imagesCopied.contains(fileName) ) {
	size = imagesCopied[fileName];
    } else {
	QString inFilePath = findDepth( fileName, imageDirList() );
	getImageSize( inFilePath, &size.first, &size.second );
	imagesCopied.insert( fileName, size );
	if ( inFilePath.isEmpty() ) {
	    warning( 1, loc, "Cannot find image file '%s'", fileName.latin1() );
	    return FALSE;
	}

	QFile fin( inFilePath );
	if ( !fin.open(IO_ReadOnly) ) {
	    message( 1, "Cannot open image file '%s'", inFilePath.latin1() );
	    return FALSE;
	}

	QByteArray img = fin.readAll();
	fin.close();

	QString outFilePath = outputDir() + QChar( '/' ) + fileName;
	QFile fout( outFilePath );
	if ( !fout.open(IO_WriteOnly) ) {
	    message( 1, "Cannot open image file '%s'", outFilePath.latin1() );
	    return FALSE;
	}
	fout.writeBlock( img );
	fout.close();
    }

    if ( width != 0 )
	*width = size.first;
    if ( height != 0 )
	*height = size.second;
    return size.first > 0 && size.second > 0;
}

QString Config::unalias( const Location& loc, const QString& alias,
			 const QString& format, const QStringList& args ) const
{
    QString key;
    QMap<QString, QString>::ConstIterator a;
    
    key = alias + QChar( '.' ) + format + QString::number( args.count() );
    a = aliasMap.find( key );
    if ( a == aliasMap.end() ) {
	key = alias + QString( ".*" ) + QString::number( args.count() );
	a = aliasMap.find( key );
	if ( a == aliasMap.end() ) {
	    warning( 1, loc, "No alias entry '%s' with %d parameter%s",
		     (alias + QChar('.') + format).latin1(), args.count(),
		     args.count() == 1 ? "" : "s" );
	    if ( args.isEmpty() )
		return "";
	    else
		return args[0];
	}
    }

    return replaceArgs( *a, args );
}

bool Config::matchLine( QString *key, QStringList *val )
{
    // key = value \n
    QRegExp keyX( QString("^[ \n\t]*([A-Z_a-z][A-Z_a-z0-9.*]*)[ \t]*=[ \t]*") );
    // there are two syntaxes for values: foo and "foo"
    QRegExp valXOrY( QString(
	    "^(?:([^\" \n\t]*)|\"((?:[^\"\\\\]|\\\\.)*)\")"
	    "(?:[ \t]|\\\\[ \t]*\n)*") );
    QRegExp backslashX( QString("\\\\(.)") );

    if ( keyX.search(yyIn.mid(yyPos)) != -1 ) {
	*key = keyX.cap( 1 );
	yyPos += keyX.matchedLength();

	while ( valXOrY.search(yyIn.mid(yyPos)) != -1 ) {
	    if ( !valXOrY.cap(1).isEmpty() ) {
		val->append( eval(valXOrY.cap(1)) );
	    } else if ( !valXOrY.cap(2).isEmpty() ) {
		QString t = valXOrY.cap( 2 );
		int k = 0;
		while ( (k = backslashX.search(t, k)) != -1 ) {
		    t.replace( k, 2, backslashX.cap(1) );
		    k += 2;
		}
		val->append( t );
	    }
	    if ( valXOrY.matchedLength() == 0 )
		break;
	    yyPos += valXOrY.matchedLength();
	}
	if ( yyIn.mid(yyPos, 1) == QChar('\n') ) {
	    yyPos++;
	    return TRUE;
	}
    }

    if ( yyPos < (int) yyIn.length() )
	message( 0, "Bad syntax in configuration file at line %d",
		 yyIn.left(yyPos).contains(QChar('\n')) + 1 );
    return FALSE;
}

void Config::showHelp()
{
    /*
      We imitate gcc output.
    */
    printf( "Usage: qdoc [options] [qdoc.conf]\n"
	    "Long options:\n"
	    "  --auto-hrefs=<yes|no>    Automatically add hrefs [%s]\n"
	    "  --base=<url>             Set documentation base URL\n"
	    "  --define=<regexp>        Define preprocessor symbols\n"
	    "  --false=<regexp>         Define false preprocessor predicates\n"
	    "  --friendly=<yes|no>      Generate filter-friendly HTML [%s]\n"
	    "  --help                   Display this information\n"
	    "  --help-short             List short options\n"
	    "  --internal=<yes|no>      Generate internal documentation [%s]\n"
	    "  --lint=<yes|no>          Report broken links [%s]\n"
	    "  --max-similar=<num>      Limit number of similar warnings [%d]\n"
	    "  --max-warnings=<num>     Limit number of warnings [%d]\n"
	    "  --only=<regexp>          Generate only matching HTML files\n"
	    "  --output-dir=<path>      Set output directory\n"
	    "  --supervisor=<yes|no>    Compare with previous run [%s]\n"
	    "  --version                Display version of qdoc\n"
	    "  --warning-level=<num>    Set warning level (0 to 4) [%d]\n",
	    toYN(autoh), toYN(frend), toYN(internal), toYN(lin), maxSim,
	    maxAll, toYN(super), wlevel );
    exit( EXIT_SUCCESS );
}

void Config::showHelpShort()
{
    printf( "Usage: qdoc [options] [qdoc.conf]\n"
	    "Short options:\n"
	    "  -a vs. -A                Automatically add hrefs [%s]\n"
	    "  -D<regexp>               Define preprocessor symbols\n"
	    "  -F<regexp>               Define false preprocessor predicates\n"
	    "  -h                       List long options\n"
	    "  -H                       Display this information\n"
	    "  -i vs. -I                Generate internal documentation [%s]\n"
	    "  -m<num>                  Limit number of similar warnings [%d]\n"
	    "  -M<num>                  Limit number of warnings [%d]\n"
	    "  -O<regexp>               Generate only matching HTML files\n"
	    "  -s vs. -S                Compare with previous run [%s]\n"
	    "  -v                       Display version of qdoc\n"
	    "  -W<num>                  Set warning level (0 to 4) [%d]\n"
	    "  -Wall                    Enable all warnings\n"
	    "  -Wnone                   Disable all warnings\n",
	    toYN(autoh), toYN(internal), maxSim, maxAll, toYN(super),
	    wlevel );
    exit( EXIT_SUCCESS );
}

void Config::showVersion()
{
    printf( "qdoc version 2.90\n" );
    exit( EXIT_SUCCESS );
}
