/*
  config.cpp
*/

#include <qdir.h>
#include <qfile.h>
#include <qregexp.h>

#include "config.h"

/*
  ###
*/
class MetaStackEntry
{
public:
    void open();
    void close();

    QStringList accum;
    QStringList next;
};

void MetaStackEntry::open()
{
    next << "";
}

void MetaStackEntry::close()
{
    accum += next;
    next.clear();
}

/*
  ###
*/
class MetaStack : private QValueStack<MetaStackEntry>
{
public:
    MetaStack();

    void process( QChar ch, const Location& location );
    QStringList getExpanded( const Location& location );
};

MetaStack::MetaStack()
{
    push( MetaStackEntry() );
    top().open();
}

void MetaStack::process( QChar ch, const Location& location )
{
    if ( ch == '{' ) {
	push( MetaStackEntry() );
	top().open();
    } else if ( ch == '}' ) {
	if ( count() == 1 )
	    location.fatal( tr("Unexpected '}'") );

	top().close();
	QStringList suffixes = pop().accum;
	QStringList prefixes = top().next;

	top().next.clear();
	QStringList::ConstIterator pre = prefixes.begin();
	while ( pre != prefixes.end() ) {
	    QStringList::ConstIterator suf = suffixes.begin();
	    while ( suf != suffixes.end() ) {
		top().next << ( *pre + *suf );
		++suf;
	    }
	    ++pre;
	}
    } else if ( ch == ',' && count() > 1 ) {
	top().close();
	top().open();
    } else {
	QStringList::Iterator pre = top().next.begin();
	while ( pre != top().next.end() ) {
	    *pre += ch;
	    ++pre;
	}
    }
}

QStringList MetaStack::getExpanded( const Location& location )
{
    if ( count() > 1 )
	location.fatal( tr("Missing '}'") );

    top().close();
    return top().accum;
}

QT_STATIC_CONST_IMPL QString Config::dot = ".";

/*!

*/
Config::Config( const QString& programName )
    : prog( programName )
{
    loc = Location::null;
    lastLoc = Location::null;
    locMap.clear();
    stringValueMap.clear();
    stringListValueMap.clear();
}

/*!

*/
void Config::load( const QString& fileName )
{
    load( Location::null, fileName );
    if ( loc.isEmpty() ) {
	loc = Location( fileName );
    } else {
	loc.setEtc( TRUE );
    }
    lastLoc = Location::null;
}

/*!

*/
void Config::setStringList( const QString& var, const QStringList& values )
{
    stringValueMap[var] = values.join( " " );
    stringListValueMap[var] = values;
}

/*!
  Returns the value of the \a var configuration variable as an
  integer.
*/
int Config::getInt( const QString& var ) const
{
    QStringList strs = getStringList( var );
    QStringList::ConstIterator s = strs.begin();
    int sum = 0;

    while ( s != strs.end() ) {
	sum += (*s).toInt();
	++s;
    }
    return sum;
}

/*!
  Returns the value of the \a var configuration variable as a string.
*/
QString Config::getString( const QString& var ) const
{
    if ( !locMap[var].isEmpty() )
	(Location&) lastLoc = locMap[var];
    return stringValueMap[var];
}

Set<QString> Config::getStringSet( const QString& var ) const
{
    return Set<QString>( getStringList(var) );
}

QStringList Config::getStringList( const QString& var ) const
{
    if ( !locMap[var].isEmpty() )
	(Location&) lastLoc = locMap[var];
    return stringListValueMap[var];
}

QRegExp Config::getRegExp( const QString& var ) const
{
    QString pattern;
    QValueList<QRegExp> subRegExps = getRegExpList( var );
    QValueList<QRegExp>::ConstIterator s = subRegExps.begin();

    while ( s != subRegExps.end() ) {
	if ( !(*s).isValid() )
	    return *s;
	if ( !pattern.isEmpty() )
	    pattern += "|";
	pattern += "(?:" + (*s).pattern() + ")";
	++s;
    }
    if ( pattern.isEmpty() )
	pattern = "$x"; // cannot match
    return QRegExp( pattern );
}

QValueList<QRegExp> Config::getRegExpList( const QString& var ) const
{
    QStringList strs = getStringList( var );
    QStringList::ConstIterator s = strs.begin();
    QValueList<QRegExp> regExps;

    while ( s != strs.end() ) {
	regExps += QRegExp( *s );
	++s;
    }
    return regExps;
}

/*!
  This function is slower than it could be.
*/
Set<QString> Config::subVars( const QString& var ) const
{
    Set<QString> result;
    QString varDot = var + ".";
    QMap<QString, QString>::ConstIterator v = stringValueMap.begin();
    while ( v != stringValueMap.end() ) {
	if ( v.key().startsWith(varDot) ) {
	    QString subVar = v.key().mid( varDot.length() );
	    int dot = subVar.find( '.' );
	    if ( dot != -1 )
		subVar.truncate( dot );
	    result.insert( subVar );
	}
	++v;
    }
    return result;
}

QStringList Config::getAllFiles( const QString& filesVar,
				 const QString& dirsVar,
				 const QString& nameFilter )
{
    QStringList result = getStringList( filesVar );
    QStringList dirs = getStringList( dirsVar );
    QStringList::ConstIterator d = dirs.begin();
    while ( d != dirs.end() ) {
	result += getFilesHere( *d, nameFilter );
	++d;
    }
    return result;
}

QString Config::findFile( const QStringList& files, const QStringList& dirs,
			  const QString& fileName )
{
    QStringList::ConstIterator f = files.begin();
    while ( f != files.end() ) {
	if ( (*f).mid((*f).findRev('/') + 1) == fileName )
	    return *f;
	++f;
    }

    QStringList::ConstIterator d = dirs.begin();
    while ( d != dirs.end() ) {
	QDir dirInfo( *d );
	if ( dirInfo.exists(fileName) )
	    return dirInfo.filePath( fileName );
	++d;
    }
    return "";
}

int Config::numParams( const QString& value )
{
    int max = 0;
    for ( int i = 0; i < (int) value.length(); i++ ) {
	if ( value[i].unicode() > 0 && value[i].unicode() < 8 )
	    max = QMAX( max, value[i].unicode() );
    }
    return max;
}

bool Config::isMetaKeyChar( QChar ch )
{
    return ch.isLetterOrNumber() || ch == '_' || ch == '.' || ch == '{'
	   || ch == '}' || ch == ',';
}

void Config::load( Location location, const QString& fileName )
{
    QRegExp keySyntax( "\\w+(?:\\.\\w+)*" );

#define SKIP_CHAR() \
	location.advance( text[i++] )
#define SKIP_SPACES() \
	while ( text[i].isSpace() && text[i] != '\n' ) \
	    SKIP_CHAR()
#define PUT_CHAR() \
	word += text[i]; \
	SKIP_CHAR();

    if ( location.depth() > 16 )
	location.fatal( tr("Too many nested includes") );

    QFile fin( fileName );
    if ( !fin.open(IO_ReadOnly) )
	location.fatal( tr("Cannot open file '%1'").arg(fileName) );

    QString text = fin.readAll();
    text += "\n\n";
    fin.close();

    location.push( fileName );
    location.start();

    int i = 0;
    while ( i < (int) text.length() ) {
	if ( text[i].isSpace() ) {
	    SKIP_CHAR();
	} else if ( text[i] == '#' ) {
	    do {
		SKIP_CHAR();
	    } while ( text[i] != '\n' );
	} else if ( isMetaKeyChar(text[i]) ) {
	    Location keyLoc = location;
	    bool plus = FALSE;
	    QString stringValue;
	    QStringList stringListValue;
	    QString word;
	    bool inQuote = FALSE;
	    bool prevWordQuoted = TRUE;
	    bool metWord = FALSE;

	    MetaStack stack;
	    do {
		stack.process( text[i], location );
		SKIP_CHAR();
	    } while ( isMetaKeyChar(text[i]) );

	    QStringList keys = stack.getExpanded( location );
	    SKIP_SPACES();

	    if ( keys.count() == 1 && keys.first() == "include" ) {
		QString includeFile;

		if ( text[i] != '(' )
		    location.fatal( tr("Bad include syntax") );
		SKIP_CHAR();
		SKIP_SPACES();
		while ( !text[i].isSpace() && text[i] != '#' ) {
		    includeFile += text[i];
		    SKIP_CHAR();
		}
		SKIP_SPACES();
		if ( text[i] != ')' )
		    location.fatal( tr("Bad include syntax") );
		SKIP_CHAR();
		SKIP_SPACES();
		if ( text[i] != '#' && text[i] != '\n' )
		    location.fatal( tr("Trailing garbage") );

		load( location,
		      QFileInfo(QFileInfo(fileName).dir(), includeFile)
		      .filePath() );
	    } else {
		if ( text[i] == '+' ) {
		    plus = TRUE;
		    SKIP_CHAR();
		}
		if ( text[i] != '=' )
		    location.fatal( tr("Expected '=' or '+=' after key") );
		SKIP_CHAR();
		SKIP_SPACES();

		for ( ;; ) {
		    if ( text[i] == '\\' ) {
			SKIP_CHAR();
			if ( text[i] == '\n' ) {
			    SKIP_CHAR();
			} else if ( text[i].unicode() > '0' &&
				    text[i].unicode() < '8' ) {
			    word += QChar( text[i].digitValue() );
			    SKIP_CHAR();
			} else {
			    PUT_CHAR();
			}
		    } else if ( text[i].isSpace() || text[i] == '#' ) {
			if ( inQuote ) {
			    if ( text[i] == '\n' )
				location.fatal( tr("Unterminated string") );
			    PUT_CHAR();
			} else {
			    if ( !word.isEmpty() ) {
				if ( metWord )
				    stringValue += " ";
				stringValue += word;
				stringListValue << word;
				metWord = TRUE;
				word = "";
				prevWordQuoted = FALSE;
			    }
			    if ( text[i] == '\n' || text[i] == '#' )
				break;
			    SKIP_SPACES();
			}
		    } else if ( text[i] == '"' ) {
			if ( inQuote ) {
			    if ( !prevWordQuoted )
				stringValue += " ";
			    stringValue += word;
			    if ( !word.isEmpty() )
				stringListValue << word;
			    metWord = TRUE;
			    word = "";
			    prevWordQuoted = TRUE;
			}
			inQuote = !inQuote;
			SKIP_CHAR();
		    } else if ( text[i] == '$' ) {
			QString var;
			SKIP_CHAR();
			while ( text[i].isLetterOrNumber() || text[i] == '_' ) {
			    var += text[i];
			    SKIP_CHAR();
			}
			if ( !var.isEmpty() ) {
			    char *val = getenv( var.latin1() );
			    if ( val == 0 ) {
				location.fatal( tr("Environment variable '%1'"
						   " undefined")
						.arg(var) );
			    } else {
				word += QString( val );
			    }
			}
		    } else {
			if ( !inQuote && text[i] == '=' )
			    location.fatal( tr("Unexpected '='") );
			PUT_CHAR();
		    }
		}

		QStringList::ConstIterator key = keys.begin();
		while ( key != keys.end() ) {
		    if ( !keySyntax.exactMatch(*key) )
			keyLoc.fatal( tr("Invalid key '%1'").arg(*key) );

		    if ( plus ) {
			if ( locMap[*key].isEmpty() ) {
			    locMap[*key] = keyLoc;
			} else {
			    locMap[*key].setEtc( TRUE );
			}
			if ( stringValueMap[*key].isEmpty() ) {
			    stringValueMap[*key] = stringValue;
			} else {
			    stringValueMap[*key] += " " + stringValue;
			}
			stringListValueMap[*key] += stringListValue;
		    } else {
			locMap[*key] = keyLoc;
			stringValueMap[*key] = stringValue;
			stringListValueMap[*key] = stringListValue;
		    }
		    ++key;
		}
	    }
	} else {
	    location.fatal( tr("Unexpected character '%1' at beginning of line")
			    .arg(text[i]) );
	}
    }
}

QStringList Config::getFilesHere( const QString& dir,
				  const QString& nameFilter )
{
    QStringList result;

    QDir dirInfo( dir );
    QStringList fileNames;
    QStringList::Iterator fn;

    dirInfo.setNameFilter( nameFilter );
    dirInfo.setSorting( QDir::Name );
    dirInfo.setFilter( QDir::Files );
    fileNames = dirInfo.entryList();
    fn = fileNames.begin();
    while ( fn != fileNames.end() ) {
	result += dirInfo.filePath( *fn );
	++fn;
    }

    dirInfo.setNameFilter( "*" );
    dirInfo.setFilter( QDir::Dirs );
    fileNames = dirInfo.entryList();
    fn = fileNames.begin();
    while ( fn != fileNames.end() ) {
	if ( *fn != "." && *fn != ".." )
	    result += getFilesHere( dirInfo.filePath(*fn), nameFilter );
	++fn;
    }
    return result;
}
