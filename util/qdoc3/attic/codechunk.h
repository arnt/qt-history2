/*
  codechunk.h
*/

#ifndef CODECHUNK_H
#define CODECHUNK_H

#include <qstring.h>

/*
  The CodeChunk class represents a tiny piece of C++ code.

  The class provides convertion between a list of lexemes and a string.  It adds
  spaces at the right place for consistent style.  The tiny pieces of code it
  represents are data types, enum values, and default parameter values.

  Apart from the piece of code itself, there are two bits of metainformation
  stored in CodeChunk: the base and the hotspot.  The base is the part of the
  piece that may be a hypertext link.  The base of

      QMap<QString, QString>

  is QMap.  When calling printHtml(), qmap.html can be specified as the link.

  The hotspot is the place the variable name should be inserted in the case of a
  variable (or parameter) declaration.  The base of

      char * []

  is between '*' and '[]'.  When calling printHtml(), the hotspot can be filled
  with arbitrary HTML.
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

    bool isEmpty() const { return s.isEmpty(); }
    QString toString() const;
    const QString& base() const { return b; }

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
