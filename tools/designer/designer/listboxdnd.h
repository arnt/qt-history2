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

#ifndef LISTBOXDND_H
#define LISTBOXDND_H

#include <qlistbox.h>
#include "listdnd.h"
#include <qlist.h>

typedef QList<QListBoxItem*> ListBoxItemList;

class ListBoxDnd : public ListDnd
{
    Q_OBJECT
public:
    // dragModes are enumerated in ListDnd
    ListBoxDnd( QListBox * eventSource, const char * name = 0 );
    
signals:
    void dropped( QListBoxItem * );
    void dragged( QListBoxItem * );

public slots:
    void confirmDrop( QListBoxItem * );

protected:
    virtual bool dropEvent( QDropEvent * event );
    virtual bool mouseMoveEvent( QMouseEvent * event );
    virtual void updateLine( const QPoint & pos );
    virtual bool canDecode( QDragEnterEvent * event );

private:
    QListBoxItem * itemAt( QPoint pos );
    int buildList( ListBoxItemList & list );
    void insertList( ListBoxItemList & list );
    void removeList( ListBoxItemList & list );
};

#endif //LISTBOXDND_H
