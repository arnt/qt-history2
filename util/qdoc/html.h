/*
  html.h
*/

#ifndef HTML_H
#define HTML_H

#include <qregexp.h>
#include <qstring.h>

inline QString htmlProtect( const QString& str, bool escapeBackslash = TRUE,
			    bool escapeQuot = FALSE )
{
    static QRegExp amp( QChar('&') ); // HTML metacharacters
    static QRegExp lt( QChar('<') );
    static QRegExp gt( QChar('>') );
    static QRegExp quot( QChar('"') );
    static QRegExp backslash( QString("\\\\") ); // qdoc metacharacter

    QString t = str;
    t.replace( amp, QString("&amp;") );
    t.replace( lt, QString("&lt;") );
    t.replace( gt, QString("&gt;") );
    if ( escapeQuot )
	t.replace( quot, QString("&quot;") );
    if ( escapeBackslash )
	t.replace( backslash, QString("&#92;") );
    return t;
}

#endif
