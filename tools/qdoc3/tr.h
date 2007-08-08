/*
  tr.h
*/

#ifndef TR_H
#define TR_H

#include <qstring.h>

inline QString tr( const char *sourceText, const char * /* comment */ = 0 )
{
    return QString( sourceText );
}

#endif
