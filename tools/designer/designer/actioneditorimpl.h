/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
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

#ifndef ACTIONEDITOR_H
#define ACTIONEDITOR_H

#include "actioneditor.h"

class QAction;

class ActionEditor : public ActionEditorBase
{
    Q_OBJECT

public:
    ActionEditor( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );

protected:
    void closeEvent( QCloseEvent *e );

protected slots:
    void accelChanged( const QString & );
    void connectionsClicked();
    void currentActionChanged( QListViewItem * );
    void deleteAction();
    void enabledChanged( bool );
    void menuTextChanged( const QString & );
    void nameChanged( const QString & );
    void newAction();
    void onChanged( bool );
    void statusTipChanged( const QString & );
    void textChanged( const QString & );
    void toggleChanged( bool );
    void toolTipChanged( const QString & );
    void whatsThisChanged( const QString & );
    void chooseIcon();

signals:
    void hidden();

private:
    void enableAll( bool enable );
    void updateEditors( QAction *a );
    
private:
    QAction *currentAction;

};

#endif // ACTIONEDITOR_H
