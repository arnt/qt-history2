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

#ifndef LISTBOXRENAME_H
#define LISTBOXRENAME_H

#include <qobject.h>
#include <qlistbox.h>

class QLineEdit;

class ListBoxRename : public QObject
{
    Q_OBJECT
public:
    ListBoxRename( QListBox * eventSource, const char * name = 0 );
    bool eventFilter( QObject *, QEvent * event );

signals:
    void itemTextChanged( const QString & );

public slots:
    void showLineEdit();
    void hideLineEdit();
    void renameClickedItem();

private:
    QListBoxItem * clickedItem;
    QListBox * src;
    QLineEdit * ed;
    bool activity;
};

#endif //LISTBOXRENAME_H
