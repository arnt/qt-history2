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

#ifndef LISTBOX_COMBO_H
#define LISTBOX_COMBO_H

#include <qvbox.h>

class QListBox;
class QLabel;

class ListBoxCombo : public QVBox
{
    Q_OBJECT

public:
    ListBoxCombo( QWidget *parent = 0, const char *name = 0 );

protected:
    QListBox *lb1, *lb2;
    QLabel *label1, *label2;

protected slots:
    void slotLeft2Right();
    void slotCombo1Activated( const QString &s );
    void slotCombo2Activated( const QString &s );

};

#endif
