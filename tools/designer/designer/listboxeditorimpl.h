/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef LISTBOXEDITORIMPL_H
#define LISTBOXEDITORIMPL_H

class QListBox;
class FormWindow;

#include "listboxeditor.h"

class ListBoxEditor : public ListBoxEditorBase
{
    Q_OBJECT

public:
    ListBoxEditor( QWidget *parent, QWidget *editWidget, FormWindow *fw );

protected slots:
    void insertNewItem();
    void deleteCurrentItem();
    void currentItemChanged( QListBoxItem * );
    void currentTextChanged( const QString & );
    void okClicked();
    void cancelClicked();
    void applyClicked();
    void choosePixmap();
    void moveItemUp();
    void moveItemDown();
    void deletePixmap();

private:
    QListBox *listbox;
    FormWindow *formwindow;
    
};

#endif
