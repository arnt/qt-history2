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

    bool match( const QString& str );
#if defined(QT_OBSOLETE)
    int match( const QString& str, int index, int *len = 0,
	       bool indexIsStart = TRUE ) const;
#endif
    int find( const QString& str, int start = 0 );
    int findRev( const QString& str, int start = -1 );
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
