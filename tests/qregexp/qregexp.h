/*
  qregexp.h
*/

#ifndef QREGEXP_H
#define QREGEXP_H

#ifndef QT_H
#include "qstring.h"
#include "qstringlist.h"
#endif

class QRegExpEngine;
struct QRegExpPrivate;

class Q_EXPORT QRegExp
{
public:
    QRegExp();
    QRegExp( const QString& pattern, bool caseSensitive = TRUE,
	     bool wildcard = FALSE, bool minimal = FALSE );
    QRegExp( const QRegExp& rx );
    ~QRegExp();
    QRegExp& operator=( const QRegExp& rx );

    bool operator==( const QRegExp& rx ) const;
    bool operator!=( const QRegExp& rx ) const { return !operator==( rx ); }

    bool isValid() const;
    QString pattern() const;
    void setPattern( const QString& pattern );
    bool caseSensitive() const;
    void setCaseSensitive( bool sensitive );
#ifndef QT_NO_REGEXP_WILDCARD
    bool wildcard() const;
    void setWildcard( bool wildcard );
#endif
    bool minimal() const;
    void setMinimal( bool minimal );

    int match( const QString& str, int start = 0 );
    int matchRev( const QString& str, int start /* ### = -1 */ );
#if 1 // ### only for testing!
#if QT_VERSION >= 255
#error Remove me in Qt 3! I am m necessary for Qt 2 tests.
#endif
    int match( const QString& str, int start, int *len,
	       bool indexIsStart = TRUE ) {
	int r = match( str, start );
	if ( len != 0 )
	    *len = matchedLength();
	return r;
    }
#endif
    bool partialMatch( const QString& str ) const;
    int matchedLength() const;
#ifndef QT_NO_REGEXP_CAPTURE
    QString capturedText( int subexpression = 0 ) const;
    QStringList capturedTexts() const;
#endif

private:
    void compile( bool caseSensitive );

    QRegExpEngine *eng;
    QRegExpPrivate *priv;
};

#endif // QREGEXP_H
