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

#ifndef BROWSER_H
#define BROWSER_H

#include <qobject.h>

class Editor;
class QTextCursor;
class QTextParagraph;
class QTextFormat;

class EditorBrowser : public QObject
{
    Q_OBJECT

public:
    EditorBrowser( Editor *e );
    ~EditorBrowser();

    bool eventFilter( QObject *o, QEvent *e );
    virtual void setCurrentEdior( Editor *e );
    virtual void addEditor( Editor *e );
    virtual bool findCursor( const QTextCursor &c, QTextCursor &from, QTextCursor &to );
    virtual void showHelp( const QString & ) {}

protected:
    Editor *curEditor;
    QTextParagraph *oldHighlightedParag;
    QString lastWord;
    QTextFormat *highlightedFormat;

};

#endif
