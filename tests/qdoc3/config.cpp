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

/*!

*/
Config::Config()
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
}

/*!

*/
void Config::setStringList( const QString& var, const QStringList& values )
{
    map[var] = values;
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
    return map[var];
}

/*!
  This function is slower than it could be.
*/
Set<QString> Config::subVars( const QString& var ) const
{
    Set<QString> result;
    QString varDot = var + ".";
    QMap<QString, QStringList>::ConstIterator m = map.begin();
    while ( m != map.end() ) {
	if ( m.key().startsWith(varDot) ) {
	    QString subVar = m.key().mid( varDot.length() );
	    int dot = subVar.find( '.' );
	    if ( dot != -1 )
		subVar.truncate( dot );
	    result.insert( subVar );
	}
	++m;
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
	result += findHere( *d, nameFilter );
	++d;
    }
    return result;
}

QString Config::findFile( const QString& filesVar, const QString& dirsVar,
			  const QString& fileName )
{
    QStringList files = getStringList( filesVar );
    QStringList::ConstIterator f = files.begin();
    while ( f != files.end() ) {
	if ( (*f).mid((*f).findRev('/') + 1) == fileName )
	    return *f;
	++f;
    }

    QStringList dirs = getStringList( dirsVar );
    QStringList::ConstIterator d = dirs.begin();
    while ( d != dirs.end() ) {
	QDir info( *d );
	if ( info.exists(fileName) )
	    return info.filePath( fileName );
	++d;
    }
    return "";
}

QString Config::dot( const QString& var, const QString& subVar )
{
    return var + "." + subVar;
}

void Config::reset()
{
    static const struct {
	const char *key;
	const char *value;
    } defs[] = {
	{ CONFIG_FORMATS, "HTML" },
	{ CONFIG_FALSEHOODS, "0" },
	{ CONFIG_SOURCELANGUAGE, "C++" },
	{ CONFIG_TABSIZE, "8" },
	{ CONFIG_TARGETLANGUAGE, "C++" },
	{ 0, 0 }
    };
    int i = 0;

    loc = Location::null;
    map.clear();
    while ( defs[i].key != 0 ) {
	map[defs[i].key].append( defs[i].value );
	i++;
    }
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

    int i = 0;
    while ( i < (int) text.length() ) {
	if ( text[i].isSpace() ) {
	    ADVANCE();
	} else if ( text[i] == '#' ) {
	    do {
		ADVANCE();
	    } while ( text[i] != '\n' );
	} else if ( text[i].isLetterOrNumber() ) {
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
		load( location, includeFile );
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
		    map[key] += value;
		} else {
		    map[key] = value;
		}
	    }
	} else {
	    Messages::fatal( location, Qdoc::tr("Bad key syntax") );
	}
    }
    depth--;
}

QStringList Config::findHere( const QString& dir, const QString& nameFilter )
{
    QStringList result;

    QDir info( dir );
    QStringList fileNames;
    QStringList::Iterator fn;

    info.setNameFilter( nameFilter );
    info.setSorting( QDir::Name );
    info.setFilter( QDir::Files );
    fileNames = info.entryList();
    fn = fileNames.begin();
    while ( fn != fileNames.end() ) {
	result += info.filePath( *fn );
	++fn;
    }

    info.setNameFilter( "*" );
    info.setFilter( QDir::Dirs );
    fileNames = info.entryList();
    fn = fileNames.begin();
    while ( fn != fileNames.end() ) {
	if ( *fn != "." && *fn != ".." )
	    result += findHere( info.filePath(*fn), nameFilter );
	++fn;
    }
    return result;
}
