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

#ifndef ICONVIEWEDITOR_H
#define ICONVIEWEDITOR_H

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
