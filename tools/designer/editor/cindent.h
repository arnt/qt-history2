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

#ifndef CINDENT_H
#define CINDENT_H

#include <private/qrichtext_p.h>

class CIndent : public QTextIndent
{
public:
    CIndent();
    virtual ~CIndent() {}
    void indent( QTextDocument *doc, QTextParagraph *parag, int *oldIndent, int *newIndent );

    void setTabSize( int ts );
    void setIndentSize( int is );
    void setAutoIndent( bool ai ) { autoIndent = ai; reindent(); }
    void setKeepTabs( bool kt ) { keepTabs = kt; }

private:
    void reindent();
    void indentLine( QTextParagraph *p, int &oldIndent, int &newIndent );
    void tabify( QString &s );

public:
    int tabSize, indentSize;
    bool autoIndent, keepTabs;
    QTextDocument *lastDoc;

};

#endif
