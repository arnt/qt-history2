/*
  htmlchunk.cpp
*/

#include <qstring.h>

#include "htmlchunk.h"

HtmlChunk::HtmlChunk()
    : len( 0 )
{
}

HtmlChunk::HtmlChunk( const QString& oldHtml )
    : len( oldHtml.length() )
{
}

HtmlChunk::HtmlChunk( const HtmlChunk& chk )
    : len( chk.len )
{
}

HtmlChunk& HtmlChunk::operator=( const HtmlChunk& chk )
{
    len = chk.len;
    return *this;
}

bool HtmlChunk::isSame( const QString& newHtml ) const
{
    return len == (int) newHtml.length();
}
