/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef BROWSER_H
#define BROWSER_H

#include <qobject.h>
#include "dlldefs.h"

class Editor;
class QTextCursor;
class QTextParagraph;
class QTextFormat;

class EDITOR_EXPORT EditorBrowser : public QObject
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
