/**********************************************************************
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
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

#ifndef LISTBOXDND_H
#define LISTBOXDND_H

#include <qobject.h>
#include <qptrlist.h>
#include <qlistbox.h>

class QWidget;
class QListBox;
typedef QPtrList<QListBoxItem> ListBoxItemList;

class ListBoxDnd : public QObject
{
    Q_OBJECT
public:
    enum DragMode { None = 0, External = 1, Internal = 2, Both = 3, Move = 4, NullDrop = 8 };

    ListBoxDnd( QListBox * eventSource, const char *name = 0 );
    void setDragMode( int mode );
    int dragMode() const;
    bool eventFilter( QObject *, QEvent * event ); 

signals:
    void dropped( QListBoxItem * );

public slots:
    void confirmDrop( QListBoxItem * );

protected:
    bool dragEnterEvent( QDragEnterEvent * event );
    bool dragLeaveEvent( QDragLeaveEvent * );
    bool dragMoveEvent( QDragMoveEvent * event );
    bool dropEvent( QDropEvent * event );
    bool mousePressEvent( QMouseEvent * event );
    bool mouseMoveEvent( QMouseEvent * event );

private:
    void updateLine( const QPoint &pos );
    QListBoxItem *itemAt( QPoint pos );
    int buildList( ListBoxItemList &list );
    void insertList( ListBoxItemList &list );
    void removeList( ListBoxItemList &list );
    QListBox *src;
    QWidget *line;
    QPoint mousePressPos;
    QPoint dragPos;
    bool dragInside;
    bool dragDelete;
    bool dropConfirmed;
    int dMode;
};

#endif //LISTBOXDND_H
