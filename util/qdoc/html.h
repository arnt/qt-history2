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
    QString t = str;
    t.replace( "&", "&amp;" );
    t.replace( '<', "&lt;" );
    t.replace( '>', "&gt;" );
    if ( escapeQuot )
	t.replace( "\"", "&quot;" );
    if ( escapeBackslash )
	t.replace( "\\", "&#92;" );
    return t;
}

#endif
