/**********************************************************************
**   Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
**   This file is part of Qt GUI Designer.
**
**   This file may be distributed under the terms of the GNU General
**   Public License version 2 as published by the Free Software
**   Foundation and appearing in the file COPYING included in the
**   packaging of this file. If you did not get the file, send email
**   to info@trolltech.com
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#ifndef MULTILINEEDITORIMPL_H
#define MULTILINEEDITORIMPL_H

#include "multilineeditor.h"

class FormWindow;
class QMultiLineEdit;

class MultiLineEditor : public MultiLineEditorBase
{
    Q_OBJECT

public:
    MultiLineEditor( QWidget *parent, QWidget *editWidget, FormWindow *fw );

protected slots:
    void okClicked();
    void applyClicked();

private:
    QMultiLineEdit *mlined;
    FormWindow *formwindow;

};

class TextEditor : public MultiLineEditorBase
{
    Q_OBJECT

public:
    TextEditor( QWidget *parent, const QString &text );

    static QString getText( QWidget *parent, const QString &text );
    
protected slots:
    void okClicked();
    void applyClicked();

};

#endif
