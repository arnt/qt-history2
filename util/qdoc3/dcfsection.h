#ifndef DCFSECTION_H
#define DCFSECTION_H

#include <qlist.h>
#include <qpair.h>
#include <qstring.h>

class QTextStream;

struct DcfSection
{
    QString title;
    QString ref;
    QList<QPair<QString, QString> > keywords;
    QList<DcfSection> *subsections;

    DcfSection() : subsections( 0 ) { }
    inline DcfSection( const DcfSection& other ) : subsections( 0 ) { operator=( other ); }
    ~DcfSection() { delete subsections; }

    inline DcfSection& operator=( const DcfSection& other )
    {
	QList<DcfSection> *oldSubsections = subsections;
	if ( other.subsections == 0 ) {
	    subsections = 0;
	} else {
	    subsections = new QList<DcfSection>( *other.subsections );
	}
	delete oldSubsections;
	title = other.title;
	ref = other.ref;
	keywords = other.keywords;
	return *this;
    }
};

inline bool operator<( const DcfSection& s1, const DcfSection& s2 ) {
    int delta = s1.title.toLower().compare( s2.title.toLower() );
    if ( delta == 0 ) {
	delta = s1.title.compare( s2.title );
	if ( delta == 0 )
	    delta = s1.ref.localeAwareCompare( s2.ref );
    }
    return delta < 0;
}

inline bool operator>( const DcfSection& s1, const DcfSection& s2 ) { return s2 < s1; }
inline bool operator<=( const DcfSection& s1, const DcfSection& s2 ) { return !( s2 < s1 ); }
inline bool operator>=( const DcfSection& s1, const DcfSection& s2 ) { return !( s1 < s2 ); }
inline bool operator==( const DcfSection& s1, const DcfSection& s2 ) { return &s1 == &s2; }
inline bool operator!=( const DcfSection& s1, const DcfSection& s2 ) { return !( s1 == s2 ); }

void appendDcfSubSection(DcfSection *dcfSect, const DcfSection &sub);
void appendDcfSubSections(DcfSection *dcfSect, const QList<DcfSection> &subs);
void generateDcfSubSections(QString indent, QTextStream &out, const DcfSection &sect);
void generateDcfSections(const DcfSection &rootSect, const QString& fileName,
                         const QString& category );

#endif
