/*
  codechunk.h
*/

#ifndef CODECHUNK_H
#define CODECHUNK_H

#include <qstring.h>

class HtmlWriter;

/*
  The CodeChunk class represents a tiny piece of C++ code.

  The class provides convertion between a list of lexemes and a string.  It adds
  spaces at the right place for consistent style. 

  The tiny pieces of code are data types, enum values, and default parameter
  values.
*/
class CodeChunk
{
public:
    CodeChunk();
    CodeChunk( const QString& str );
    CodeChunk( const CodeChunk& chk );

    CodeChunk& operator=( const CodeChunk& chk );

    void append( const QString& lexeme );
    void appendBase( const QString& lexeme );
    void appendHotspot();

    const QString& toString() const { return s; }
    const QString& base() const { return b; }

    void printHtml( HtmlWriter& out,
		    const QString& baseHref = QString::null,
		    const QString& hotspotHtml = QString::null ) const;

private:
    QString s;
    QString b;
    int bstart;
    int blen;
    int hspot;
};

inline bool operator==( const CodeChunk& c, const CodeChunk& d ) {
    return c.toString() == d.toString();
}

inline bool operator!=( const CodeChunk& c, const CodeChunk& d ) {
    return !( c == d );
}

inline bool operator<( const CodeChunk& c, const CodeChunk& d ) {
    return c.toString() < d.toString();
}

inline bool operator>( const CodeChunk& c, const CodeChunk& d ) {
    return d < c;
}

inline bool operator<=( const CodeChunk& c, const CodeChunk& d ) {
    return !( c > d );
}

inline bool operator>=( const CodeChunk& c, const CodeChunk& d ) {
    return !( c < d );
}

#endif
