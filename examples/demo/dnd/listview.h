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

#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <q3listview.h>

class ListViewItem : public Q3ListViewItem
{
public:
    ListViewItem ( Q3ListView * parent, const QString& name, const QString& tag )
        : Q3ListViewItem( parent, name ), _tag( tag ) {}
    ListViewItem ( Q3ListView * parent, Q3ListViewItem * after, const QString& name, const QString& tag )
        : Q3ListViewItem( parent, after, name ), _tag( tag ) {}
    virtual ~ListViewItem() {}

    QString tag() { return _tag; }

private:
    QString _tag;
};

class ListView : public Q3ListView
{
    Q_OBJECT

public:
    ListView( QWidget* parent = 0, const char* name = 0 );
    ~ListView();

    void dragEnterEvent( QDragEnterEvent * );
    void dropEvent( QDropEvent * );
    void contentsMousePressEvent( QMouseEvent * );
    void contentsMouseMoveEvent( QMouseEvent * );
    void contentsMouseReleaseEvent( QMouseEvent * );

private:
    QPoint pressPos;
    bool dragging;
};

#endif // LISTVIEW_H
