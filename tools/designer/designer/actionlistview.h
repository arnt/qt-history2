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

#ifndef ACTIONLISTVIEW_H
#define ACTIONLISTVIEW_H

#include <qlistview.h>
#include "actiondnd.h"

class ActionItem : public QListViewItem
{
public:
    ActionItem( QListView *lv, bool group )
	: QListViewItem( lv ),
	  a( group ? 0 : new QDesignerAction( 0 ) ),
	  g( group ? new QDesignerActionGroup( 0 ) : 0 ) { setDragEnabled( TRUE ); }
    ActionItem( QListView *lv, QAction *ac );
    ActionItem( QListViewItem *i, QAction *ac );
    ActionItem( ActionItem *parent, bool group = FALSE )
	: QListViewItem( parent ),
	  a( group ? 0 : new QDesignerAction( parent->actionGroup() ) ),
	  g( group ? new QDesignerActionGroup( parent->actionGroup() ) : 0 ) { setDragEnabled( TRUE ); moveToEnd(); }

    QDesignerAction *action() const { return a; }
    QDesignerActionGroup *actionGroup() const { return g; }

private:
    void moveToEnd();

private:
    QDesignerAction *a;
    QDesignerActionGroup *g;

};

class ActionListView : public QListView
{
    Q_OBJECT

public:
    ActionListView( QWidget *parent = 0, const char *name = 0 );

protected:
    QDragObject *dragObject();

private slots:
    void rmbMenu( QListViewItem *i, const QPoint &p );

signals:
    void insertAction();
    void insertActionGroup();
    void insertDropDownActionGroup();
    void deleteAction();
    void connectAction();

};

#endif
