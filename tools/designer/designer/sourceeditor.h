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

#ifndef SOURCEEDITOR_H
#define SOURCEEDITOR_H

#include "../interfaces/editorinterface.h"
#include <qvbox.h>
#include <qguardedptr.h>

class FormWindow;
class QCloseEvent;
struct LanguageInterface;
class Project;
class SourceFile;

class SourceEditor : public QVBox
{
    Q_OBJECT

public:
    SourceEditor( QWidget *parent, EditorInterface *iface, LanguageInterface *liface );
    ~SourceEditor();

    void setObject( QObject *fw, Project *p );
    QObject *object() const { return obj; }
    Project *project() const { return pro; }
    void setFunction( const QString &func, const QString &clss = QString::null );
    void setClass( const QString &clss );
    void save();
    bool saveAs();
    void setModified( bool b );
    bool isModified() const;

    static QString sourceOfObject( QObject *fw, const QString &lang, EditorInterface *iface, LanguageInterface *lIface );

    QString language() const;
    void setLanguage( const QString &l );

    void editCut();
    void editCopy();
    void editPaste();
    bool editIsUndoAvailable();
    bool editIsRedoAvailable();
    void editUndo();
    void editRedo();
    void editSelectAll();

    void configChanged();
    void refresh( bool allowSave );
    void resetContext();

    EditorInterface *editorInterface() const { return iFace; }

    void setFocus();
    int numLines() const;
    void saveBreakPoints();
    void clearStep();
    void clearStackFrame();
    void resetBreakPoints();

    QString text() const;

    void checkTimeStamp();

    SourceFile *sourceFile() const;
    FormWindow *formWindow() const;

protected:
    void closeEvent( QCloseEvent *e );


private:
    EditorInterface *iFace;
    LanguageInterface *lIface;
    QGuardedPtr<QObject> obj;
    Project *pro;
    QString lang;
    QGuardedPtr<QWidget> editor;

};

#endif
