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

#ifndef CPPEDITOR_H
#define CPPEDITOR_H

#include <editor.h>

class EditorCompletion;
class EditorBrowser;
struct DesignerInterface;
class CIndent;

class  CppEditor : public Editor
{
    Q_OBJECT

public:
    CppEditor( const QString &fn, QWidget *parent, const char *name, DesignerInterface *i );
    ~CppEditor();

    virtual EditorCompletion *completionManager() { return completion; }
    virtual EditorBrowser *browserManager() { return browser; }
    void configChanged();

    bool supportsBreakPoints() const { return FALSE; }
#if defined(Q_USING)
    using QTextEdit::createPopupMenu;
#endif
    QPopupMenu *createPopupMenu( const QPoint &p );

    void paste();

private slots:
    void addInclDecl();
    void addInclImpl();
    void addForward();

protected:
    EditorCompletion *completion;
    EditorBrowser *browser;
    DesignerInterface *dIface;
    CIndent *indent;

};

#endif
