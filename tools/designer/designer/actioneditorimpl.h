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

#ifndef ACTIONEDITORIMPL_H
#define ACTIONEDITORIMPL_H

#include "actioneditor.h"

class QAction;
class FormWindow;
class ActionItem;

class ActionEditor : public ActionEditorBase
{
    Q_OBJECT

public:
    ActionEditor( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    void setFormWindow( FormWindow *fw );
    void updateActionName( QAction *a );
    void updateActionIcon( QAction *a );
    FormWindow *form() const { return formWindow; }

    bool wantToBeShown() const { return !explicitlyClosed; }
    void setWantToBeShown( bool b ) { explicitlyClosed = !b; }

    void setCurrentAction( QAction *a );
    QAction *newActionEx(); //FIXME: rename. mmonsen 21112002.

protected:
    void closeEvent( QCloseEvent *e );

protected slots:
    void currentActionChanged( QListViewItem * );
    void deleteAction();
    void newAction();
    void newActionGroup();
    void newDropDownActionGroup();
    void connectionsClicked();

signals:
    void hidden();
    void removing( QAction * );

private:
    void insertChildActions( ActionItem *i );
    void removeAction( QAction *a );

private:
    QAction *currentAction;
    FormWindow *formWindow;
    bool explicitlyClosed;

};

#endif // ACTIONEDITORIMPL_H
