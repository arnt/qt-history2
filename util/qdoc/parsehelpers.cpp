/*
  parsehelpers.cpp
*/

#include <qstring.h>

#include "binarywriter.h"
#include "html.h"
#include "location.h"
#include "messages.h"
#include "parsehelpers.h"

// see also doc.cpp
static QString punctuation( ".,:;" );

QString OpenedList::beginHtml()
{
    if ( kind == Bullet ) {
	return QString( "<ul>" );
    } else {
	QString ol( "<ol type=" );

	switch ( kind ) {
	case Arabic:
	    ol += QChar( '1' );
	    break;
	case Uppercase:
	    ol += QChar( 'A' );
	    break;
	case Lowercase:
	    ol += QChar( 'a' );
	    break;
	default:
	    ;
	}

	if ( nextItem != 1 )
	    ol += QString( " start=%1" ).arg( nextItem );
	ol += QChar( '>' );
	return ol;
    }
}

QString OpenedList::itemHtml()
{
    nextItem++;
    return QString( "<li>" );
}

QString OpenedList::endHtml()
{
    return kind == Bullet ? QString( "</ul>" ) : QString( "</ol>" );
}

QString OpenedList::beginSgml()
{
    if ( kind == Bullet ) {
	return QString( "<itemizedlist>" );
    } else {
	QString ol( "<orderedlist numeration=" );

	switch ( kind ) {
	case Arabic:
	    ol += QString( "arabic" );
	    break;
	case Uppercase:
	    ol += QString( "upperalpha" );
	    break;
	case Lowercase:
	    ol += QString( "loweralpha" );
	    break;
	default:
	    ;
	}

#if 0
	if ( nextItem != 1 )
	    ol += QString( " start=%1" ).arg( nextItem );
#endif
	ol += QChar( '>' );
	return ol;
    }
}

QString OpenedList::itemSgml()
{
    nextItem++;
    return QString( "<listitem>" );
}

QString OpenedList::endSgml()
{
    return kind == Bullet ? QString( "</itemizedlist>" )
		   : QString( "</orderedlist>" );
}

OpenedList openList( const Location& loc, const QString& spec )
{
    if ( spec.isEmpty() )
	return OpenedList();

    bool ok;
    int n = spec.toInt( &ok );
    int ch = spec[0].unicode();

    if ( ok ) {
	return OpenedList( OpenedList::Arabic, n );
    } else if ( spec.length() == 1 && ch >= 'A' && ch <= 'Z' ) {
	return OpenedList( OpenedList::Uppercase, ch - 'A' + 1 );
    } else if ( spec.length() == 1 && ch >= 'a' && ch <= 'z' ) {
	return OpenedList( OpenedList::Uppercase, ch - 'a' + 1 );
    } else {
	warning( 1, loc, "Unknown list specification '%s'", spec.latin1() );
	return OpenedList( OpenedList() );
    }
}

void SectionNumber::advance( int level )
{
    if ( level < (int) number.count() ) {
	number[level].setNum( number[level].toInt() + 1 );
	while ( level < (int) number.count() - 1 )
	    number.remove( number.at((int) number.count() - 1) );
    } else {
	while ( level < (int) number.count() - 1 )
	    number.append( QString("0") );
	number.append( QString("1") );
    }
}

QString SectionNumber::fileSuffix( int granularity ) const
{
    QString t;

    for ( int i = 0; i <= granularity && i < (int) number.count(); i++ )
	t += QChar( '-' ) + number[i];
    return t;
}

QString SectionNumber::target( int granularity ) const
{
    QString t;
    for ( int i = granularity + 1; i < (int) number.count(); i++ )
	t += QChar( '-' ) + number[i];
    if ( !t.isEmpty() )
	t = t.mid( 1 );
    return t;
}

static bool ellipsisMatch( const QString& ellipsis, const QString& target )
{
    QString low = target.lower();

    int k = ellipsis.find( QString("...") );
    if ( k == -1 ) {
	return ellipsis.lower() == low;
    } else {
	return low.startsWith( ellipsis.left(k).lower() ) &&
	       low.endsWith( ellipsis.mid(k + 3).lower() );
    }
}

QValueList<Section *> recursiveSectionResolve( QValueList<Section> *sects,
					       const QStringList& toks )
{
    QValueList<Section *> candidates;

    QValueList<Section>::Iterator s = sects->begin();
    while ( s != sects->end() ) {
	candidates += recursiveSectionResolve( (*s).subsections(), toks );

	if ( ellipsisMatch(toks.first(), (*s).title) ) {
	    if ( toks.count() == 1 ) {
		candidates.push_back( &*s );
	    } else {
		QStringList subToks = toks;
		subToks.remove( subToks.begin() );
		candidates += recursiveSectionResolve( (*s).subsections(),
						       subToks );
	    }
	}
	++s;
    }
    return candidates;
}

void appendXmlSubSection( XmlSection *xmlSect, const XmlSection& sub )
{
    if ( xmlSect->subsections == 0 )
	xmlSect->subsections = new QValueList<XmlSection>;
    xmlSect->subsections->append( sub );
}

void sortXmlSubSections( XmlSection *xmlSect )
{
    if ( xmlSect->subsections != 0 )
	qHeapSort( *xmlSect->subsections );
}

void generateXmlSubSections( QString indent, BinaryWriter& out,
			     const XmlSection& sect )
{
    indent += "    ";
    QValueList<XmlSection>::ConstIterator ss = sect.subsections->begin();
    while ( ss != sect.subsections->end() ) {
	out.puts( indent + "<section ref=\"" + htmlProtect( (*ss).ref, FALSE ) +
		  "\" title=\"" + htmlProtect( (*ss).title, FALSE ) + "\"" );
	if ( (*ss).keywords.isEmpty() && (*ss).subsections == 0 ) {
	    out.puts( "/>\n" );
	} else {
	    out.puts( ">\n" );
	    // output the keyword
	    if ( (*ss).subsections != 0 )
		generateXmlSubSections( indent, out, *ss );
	    out.puts( indent + "</section>\n" );
	}
	++ss;
    }
}

void generateXmlSections( const XmlSection& rootSect, const QString& fileName,
			  const QString& category )
{
    BinaryWriter out( fileName );

    out.puts( "<!DOCTYPE DOC>\n" );
    out.puts( "<DOC ref=\"" + htmlProtect(rootSect.ref, FALSE) +
	      "\" category=\"" + htmlProtect(category, FALSE) + "\" title=\"" +
	      htmlProtect(rootSect.title, FALSE) + "\">\n" );

    generateXmlSubSections( "", out, rootSect );

    out.puts( "</DOC>\n" );
}

void skipSpaces( const QString& in, int& pos )
{
    while ( pos < (int) in.length() && in[pos].isSpace() &&
	    in[pos].unicode() != '\n' )
	pos++;
}

/*
  This function is highly magical. It tries to somehow reproduce the
  old qdoc behavior.
*/
void skipSpacesOrNL( const QString& in, int& pos )
{
    int firstNL = -1;
    while ( pos < (int) in.length() && in[pos].isSpace() ) {
	QChar ch = in[pos];
	if ( ch == QChar('\n') ) {
	    if ( firstNL == -1 ) {
		firstNL = pos;
	    } else {
		pos = firstNL;
		break;
	    }
	}
	pos++;
    }
}

void skipRestOfLine( const QString& in, int& pos )
{
    while ( pos < (int) in.length() && in[pos] != '\n' )
	pos++;
}

QString getWord( const QString& in, int& pos )
{
    skipSpaces( in, pos );
    int begin = pos;
    while ( pos < (int) in.length() && !in[pos].isSpace()
	    && in[pos].unicode() != '\\' )
	pos++;
    return in.mid( begin, pos - begin );
}

QString getRestOfLine( const QString& in, int& pos )
{
    skipSpaces( in, pos );
    int begin = pos;
    while ( pos < (int) in.length() && in[pos].unicode() != '\n' )
	pos++;

    QString t = in.mid( begin, pos - begin ).simplifyWhiteSpace();
    skipSpacesOrNL( in, pos );
    return t;
}

QString getRestOfParagraph( const QString& in, int& pos )
{
    int numNLsInRow = 0;

    skipSpaces( in, pos );
    int begin = pos;
    while ( pos < (int) in.length() && numNLsInRow < 2 ) {
	QChar ch = in[pos];
	if ( ch.unicode() == '\n' ) {
	    numNLsInRow++;
	} else if ( !ch.isSpace() ) {
	    if ( numNLsInRow == 1 && ch == QChar('\\') )
		break;
	    numNLsInRow = 0;
	}
	pos++;
    }

    QString t = in.mid( begin, pos - begin ).simplifyWhiteSpace();
    skipSpacesOrNL( in, pos );
    return t;
}

QString getPrototype( const QString& in, int& pos )
{
    skipSpaces( in, pos );
    int begin = pos;
    int level = 0;
    int ch;
    while ( pos < (int) in.length() &&
	    ((ch = in[pos].unicode()) != '\n' || level > 0) )
    {
	if ( ch == '(' )
	    level++;
	else if ( ch == ')' )
	    level--;
	pos++;
    }

    QString t = in.mid( begin, pos - begin ).simplifyWhiteSpace();
    skipSpaces( in, pos );
    return t;
}

QString getArgument( const QString& in, int& pos )
{
    int parenDepth = 0;
    int bracketDepth = 0;

    skipSpacesOrNL( in, pos );
    int begin = pos;

    /*
      Typically, an argument ends at the next white-space. However,
      braces can be used to group words:

	  {a few words}

      Also, opening and closing parentheses have to match. Thus,

	  printf( "%d\n", x )

      is an argument too, although it contains spaces. Finally,
      trailing punctuation is not included in an argument, nor is 's.
    */
    if ( pos < (int) in.length() && in[pos] == QChar('{') ) {
	pos++;
	while ( pos < (int) in.length() && in[pos] != QChar('}') )
	    pos++;
	pos++;
	return in.mid( begin + 1, pos - begin - 2 ).simplifyWhiteSpace();
    } else {
	begin = pos;
	while ( pos < (int) in.length() ) {
	    QChar ch = in[pos];

	    switch ( ch.unicode() ) {
	    case '(':
		parenDepth++;
		break;
	    case ')':
		parenDepth--;
		break;
	    case '[':
		bracketDepth++;
		break;
	    case ']':
		bracketDepth--;
	    }
	    if ( parenDepth < 0 || bracketDepth < 0 || ch == QChar('\\') )
		break;
	    if ( ch.isSpace() && parenDepth <= 0 && bracketDepth <= 0 )
		break;
	    pos++;
	}

	if ( pos > begin + 1 && punctuation.find(in[pos - 1]) != -1 &&
	     in.mid(pos - 3, 3) != QString("...") )
	    pos--;

	if ( pos > begin + 2 && in[pos - 2] == QChar('\'') &&
	     in[pos - 1] == QChar('s') )
	    pos -= 2;
	return in.mid( begin, pos - begin ).simplifyWhiteSpace();
    }
}
