/*
  parsehelpers.h
*/

#ifndef PARSEHELPERS_H
#define PARSEHELPERS_H

#include <qpair.h>
#include <qstringlist.h>
#include <qvaluelist.h>

class Location;

class QString;

class OpenedList
{
public:
    enum Kind { Bullet, Arabic, Uppercase, Lowercase };

    OpenedList( Kind k = Bullet, int first = 1 )
	: kind( k ), nextItem( first ) { }

    QString beginHtml();
    QString itemHtml();
    QString endHtml();

    QString beginSgml();
    QString itemSgml();
    QString endSgml();

private:
    Kind kind;
    int nextItem;
};

OpenedList openList( const Location& loc, const QString& spec );

struct SectionNumber
{
    QValueList<QString> number;

    void advance( int level );

    QString fileSuffix( int granularity = -1 ) const;
    QString target( int granularity = -1 ) const;
};

struct Section
{
    SectionNumber number;
    QString title;
    QValueList<Section> *subs;

    Section() : subs( 0 ) { }
    Section( const Section& s ) : subs( 0 ) { operator=( s ); }
    ~Section() { delete subs; }

    Section& operator=( const Section& s ) {
	number = s.number;
	title = s.title;
	if ( subs == 0 )
	    subs = new QValueList<Section>;
	*subs = *s.subsections();
	return *this;
    }

    QValueList<Section> *subsections() const {
	if ( subs == 0 )
	    ((Section *) this)->subs = new QValueList<Section>;
	return subs;
    }
};

QValueList<Section *> recursiveSectionResolve( QValueList<Section> *sects,
					       const QStringList& toks );

struct XmlSection
{
    QString title;
    QString ref;
    QValueList<QPair<QString, QString> > keywords;
    QValueList<XmlSection> *subsections;

    XmlSection() : subsections( 0 ) { }
    XmlSection( const XmlSection& other ) : subsections( 0 ) {
	operator=( other );
    }
    ~XmlSection() { delete subsections; }

    XmlSection& operator=( const XmlSection& other ) {
	delete subsections;
	if ( other.subsections == 0 ) {
	    subsections = 0;
	} else {
	    subsections = new QValueList<XmlSection>( *other.subsections );
	}
	title = other.title;
	ref = other.ref;
	keywords = other.keywords;
	return *this;
    }

    bool operator>( const XmlSection& other ) {
	return title > other.title;
    }

    bool operator<=( const XmlSection& other ) {
	return !operator>(other);
    }
};

inline bool operator<( const XmlSection& s1, const XmlSection& s2 )
{
    int delta = s1.title.lower().compare( s2.title.lower() );
    if ( delta == 0 ) {
	delta = s1.title.compare( s2.title );
	if ( delta == 0 )
	    delta = s1.ref.localeAwareCompare( s2.ref );
    }
    return delta < 0;
}

class BinaryWriter;

void appendXmlSubSection( XmlSection *xmlSect, const XmlSection& sub );
void sortXmlSubSections( XmlSection *xmlSect );
void generateXmlSubSections( QString indent, BinaryWriter& out,
			     const XmlSection& sect );
void generateXmlSections( const XmlSection& rootSect, const QString& fileName,
			  const QString& category );

/*
  HASH() combines a character (the first character of a word) and a
  length to form a hash value.

  CHECK() and CONSUME() compare variable 'command' with a target
  string. If they are not equal, break. If they are equal,
  CONSUME(), unlike CHECK(), removes the '\command' from the text.
*/

#define HASH( ch, len ) ( (ch) | ((len) << 8) )
#define CHECK( target ) \
    if ( strcmp(target, command.latin1()) != 0 ) \
	break
#define CONSUME( target ) \
    CHECK( target ); \
    consumed = TRUE

void skipSpaces( const QString& in, int& pos );
void skipSpacesOrNL( const QString& in, int& pos );
void skipRestOfLine( const QString& in, int& pos );
QString getWord( const QString& in, int& pos );
QString getRestOfLine( const QString& in, int& pos );
QString getRestOfParagraph( const QString& in, int& pos );
QString getPrototype( const QString& in, int& pos );
QString getArgument( const QString& in, int& pos );

#endif
