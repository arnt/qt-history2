/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SYNTAXHIGHLIGHTER_HTML_H
#define SYNTAXHIGHLIGHTER_HTML_H

#include <./private/qrichtext_p.h>

class SyntaxHighlighter_HTML : public Q3TextPreProcessor
{
public:

    enum HTML {
	Standard = 1,
	Keyword,
	Attribute,
	AttribValue
    };

    SyntaxHighlighter_HTML();
    virtual ~SyntaxHighlighter_HTML();
    void process( Q3TextDocument *doc, Q3TextParagraph *string, int start, bool invalidate = TRUE );
    Q3TextFormat *format( int id );

private:
    void addFormat( int id, Q3TextFormat *f );

    Q3TextFormat *lastFormat;
    int lastFormatId;
    QHash<int, Q3TextFormat*> formats;

};

#endif
