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

#ifndef SOURCEEDITOR_H
#define SOURCEEDITOR_H

#include "../interfaces/editorinterface.h"
#include <qvbox.h>

class FormWindow;
class QCloseEvent;
struct LanguageInterface;

class SourceEditor : public QVBox
{
    Q_OBJECT

public:
    SourceEditor( QWidget *parent, EditorInterface *iface, LanguageInterface *liface );
    ~SourceEditor();

    void setForm( FormWindow *fw );
    void setFunction( const QString &func );
    void save();

    static QString sourceOfForm( FormWindow *fw, const QString &lang, EditorInterface *iface, LanguageInterface *lIface );

    QString language() const;
    void setLanguage( const QString &l );

    void editCut();
    void editCopy();
    void editPaste();
    void editUndo();
    void editRedo();
    void editSelectAll();

protected:
    void closeEvent( QCloseEvent *e );

signals:
    void hidden();

private:
    EditorInterface *iFace;
    LanguageInterface *lIface;
    FormWindow *formWindow;
    QString lang;

};

#endif
