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

class SyntaxHighlighter_HTML : public QTextPreProcessor
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
    void process( QTextDocument *doc, QTextParagraph *string, int start, bool invalidate = TRUE );
    QTextFormat *format( int id );

private:
    void addFormat( int id, QTextFormat *f );

    QTextFormat *lastFormat;
    int lastFormatId;
    QIntDict<QTextFormat> formats;

};

#endif
