/*
  htmlchunk.h
*/

#ifndef HTMLCHUNK_H
#define HTMLCHUNK_H

class QString;

class HtmlChunk
{
public:
    HtmlChunk();
    HtmlChunk( const QString& oldHtml );
    HtmlChunk( const HtmlChunk& chk );

    HtmlChunk& operator=( const HtmlChunk& chk );

    bool isSame( const QString& newHtml ) const;
    int length() const { return len; }

private:
    int len;
};

#endif
