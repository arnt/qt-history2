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

/*
  What is sameness?  Some people observed that you cannot define sameness before
  defining change, yet that you cannot define change before you define sameness.
  We will leave that debate to philosophers (and lunatics) and say that two HTML
  chunks are the same if they have the same length.  This is a rather weak
  definition, and could be strengthened with hashing in some future version of
  qdoc.
*/
bool HtmlChunk::isSame( const QString& newHtml ) const
{
    return len == (int) newHtml.length();
}
