/*
  config.cpp
*/

#include <qdir.h>
#include <qfile.h>
#include <qregexp.h>

#include "config.h"
#include "messages.h"

/*! \class Config

*/

QT_STATIC_CONST_IMPL QString Config::dot = ".";

/*!

*/
Config::Config( const QString& programName )
    : prog( programName )
{
    reset();
}

/*!

*/
void Config::load( const QString& fileName )
{
    reset();
    load( Location::null, fileName );
    loc = Location( fileName );
    lastLoc = Location( fileName );
}

/*!

*/
void Config::setStringList( const QString& var, const QStringList& values )
{
    valueMap[var] = values;
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
    return getStringList( var ).join( " " );
}

Set<QString> Config::getStringSet( const QString& var ) const
{
    return Set<QString>( getStringList(var) );
}

QStringList Config::getStringList( const QString& var ) const
{
    if ( !locMap[var].isEmpty() )
	(Location&) lastLoc = locMap[var];
    return valueMap[var];
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
    QMap<QString, QStringList>::ConstIterator v = valueMap.begin();
    while ( v != valueMap.end() ) {
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

void Config::reset()
{
    loc = Location::null;
    lastLoc = Location::null;
    locMap.clear();
    valueMap.clear();
}

void Config::load( Location location, const QString& fileName )
{
#define ADVANCE() \
	location.advance( text[i++] )
#define SKIP_SPACES() \
	while ( text[i].isSpace() && text[i] != '\n' ) \
	    ADVANCE()

    static int depth = 0;

    if ( depth++ > 16 )
	Messages::fatal( location, Qdoc::tr("Too many nested includes") );

    QFile fin( fileName );
    if ( !fin.open(IO_ReadOnly) )
	Messages::fatal( location,
			 Qdoc::tr("Cannot open file '%1'").arg(fileName) );

    QString text = fin.readAll();
    text += "\n\n";
    fin.close();

    location.push( fileName );
    location.start();

    int i = 0;
    while ( i < (int) text.length() ) {
	if ( text[i].isSpace() ) {
	    ADVANCE();
	} else if ( text[i] == '#' ) {
	    do {
		ADVANCE();
	    } while ( text[i] != '\n' );
	} else if ( text[i].isLetterOrNumber() ) {
	    Location keyLoc = location;
	    QRegExp keySyntax( "\\w+(?:\\.\\w+)*" );
	    QString key;
	    bool plus = FALSE;
	    QStringList value;
	    bool inQuote = FALSE;

	    do {
		key += text[i];
		ADVANCE();
	    } while ( text[i].isLetterOrNumber() || text[i] == '_' ||
		      text[i] == '.' );

	    if ( !keySyntax.exactMatch(key) )
		Messages::fatal( location, Qdoc::tr("Bad key syntax") );

	    SKIP_SPACES();

	    if ( key == "include" ) {
		QString includeFile;

		if ( text[i] != '(' )
		    Messages::fatal( location, Qdoc::tr("Bad include syntax") );
		ADVANCE();
		SKIP_SPACES();
		while ( !text[i].isSpace() && text[i] != '#' ) {
		    includeFile += text[i];
		    ADVANCE();
		}
		SKIP_SPACES();
		if ( text[i] != ')' )
		    Messages::fatal( location, Qdoc::tr("Bad include syntax") );
		ADVANCE();
		SKIP_SPACES();
		if ( text[i] != '#' && text[i] != '\n' )
		    Messages::fatal( location, Qdoc::tr("Trailing garbage") );

		load( location,
		      QFileInfo(QFileInfo(fileName).dir(), includeFile)
		      .filePath() );
	    } else {
		if ( text[i] == '+' ) {
		    plus = TRUE;
		    ADVANCE();
		}
		if ( text[i] != '=' )
		    Messages::fatal( location, Qdoc::tr("Expected '=' or '+='"
							" after key") );
		ADVANCE();
		SKIP_SPACES();
		value.append( "" );

		while ( text[i] != '#' && text[i] != '\n' ) {
		    if ( text[i] == '\\' ) {
			ADVANCE();
			if ( text[i] == '\n' ) {
			    ADVANCE();
			} else {
			    value.last().append( text[i] );
			    ADVANCE();
			}
		    } else if ( text[i].isSpace() ) {
			if ( inQuote ) {
			    value.last().append( text[i] );
			    ADVANCE();
			} else {
			    value.append( "" );
			    SKIP_SPACES();
			}
		    } else if ( text[i] == '"' ) {
			value.append( "" );
			inQuote = !inQuote;
			ADVANCE();
		    } else if ( text[i] == '$' ) {
			QString var;
			ADVANCE();
			while ( text[i].isLetterOrNumber() || text[i] == '_' ) {
			    var += text[i];
			    ADVANCE();
			}
			if ( !var.isEmpty() ) {
			    char *val = getenv( var.latin1() );
			    if ( val == 0 ) {
				Messages::fatal( location,
						 Qdoc::tr("Environment variable"
							  " '%1' undefined")
						 .arg(var) );
			    } else {
				value.last().append( QString(val) );
			    }
			}
		    } else {
			if ( !inQuote && text[i] == '=' )
			    Messages::fatal( location,
					     Qdoc::tr("Unexpected '='") );
			value.last().append( text[i] );
			ADVANCE();
		    }
		}
		if ( inQuote )
		    Messages::fatal( location,
				     Qdoc::tr("Unterminated string") );
		value.remove( "" );

		if ( plus ) {
		    if ( locMap[key].isEmpty() ) {
			locMap[key] = keyLoc;
		    } else {
			locMap[key].setEtc( TRUE );
		    }
		    valueMap[key] += value;
		} else {
		    locMap[key] = keyLoc;
		    valueMap[key] = value;
		}
	    }
	} else {
	    Messages::fatal( location, Qdoc::tr("Bad key syntax") );
	}
    }
    depth--;
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
