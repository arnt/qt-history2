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

#ifndef ICONVIEWEDITORIMPL_H
#define ICONVIEWEDITORIMPL_H

#include "iconvieweditor.h"

class FormWindow;

class IconViewEditor : public IconViewEditorBase
{
    Q_OBJECT

public:
    IconViewEditor( QWidget *parent, QWidget *editWidget, FormWindow *fw );

protected slots:
    void insertNewItem();
    void deleteCurrentItem();
    void currentItemChanged( QIconViewItem * );
    void currentTextChanged( const QString & );
    void okClicked();
    void cancelClicked();
    void applyClicked();
    void choosePixmap();
    void deletePixmap();

private:
    QIconView *iconview;
    FormWindow *formwindow;
    
};

#endif
