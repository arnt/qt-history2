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

#ifndef EDITOR_H
#define EDITOR_H

#include <qtextedit.h>
#include "dlldefs.h"
#include <qrichtext_p.h>

struct Config;
class ParenMatcher;
class EditorCompletion;

class EDITOR_EXPORT Editor : public QTextEdit
{
    Q_OBJECT

public:
    enum Selection {
	Error = QTextDocument::Selection4,
	Step = QTextDocument::Selection5
    };

    Editor( const QString &fn, QWidget *parent, const char *name );
    virtual void load( const QString &fn );
    virtual void save( const QString &fn );
    QTextDocument *document() const { return QTextEdit::document(); }
    void setDocument( QTextDocument *doc ) { QTextEdit::setDocument( doc ); }
    QTextCursor *textCursor() const { return QTextEdit::textCursor(); }

    virtual EditorCompletion *completionManager() { return 0; }
    virtual void configChanged();

    Config *config() { return cfg; }

    void setErrorSelection( int line );
    void setStepSelection( int line );
    void clearSelections();

    virtual bool supportsErrors() const { return TRUE; }
    virtual bool supportsBreakPoints() const { return TRUE; }

signals:
    void clearErrorMarker();

private slots:
    void cursorPosChanged( QTextCursor *c );

protected:
    ParenMatcher *parenMatcher;
    QString filename;
    Config *cfg;
    bool hasError;

};

#endif
