/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCPPSYNTAXHIGHLIGHTER_H
#define QCPPSYNTAXHIGHLIGHTER_H

#include <qhash.h>
#include <private/qrichtext_p.h>
#include <conf.h>

class SyntaxHighlighter_CPP : public Q3TextPreProcessor
{
public:
    enum CppIds {
	Comment = 1,
	Number,
	String,
	Type,
	Keyword,
	PreProcessor,
	Label
    };

    SyntaxHighlighter_CPP();
    virtual ~SyntaxHighlighter_CPP();
    void process( Q3TextDocument *doc, Q3TextParagraph *string, int start, bool invalidate = TRUE );
    void updateStyles( const QMap<QString, ConfigStyle> &styles );

    static const char * const keywords[];

    Q3TextFormat *format( int id );

private:
    void addFormat( int id, Q3TextFormat *f );
    void removeFormat( int id );

    Q3TextFormat *lastFormat;
    int lastFormatId;
    QHash<int, Q3TextFormat*> formats;

};

#endif
