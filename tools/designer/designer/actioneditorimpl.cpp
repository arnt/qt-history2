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

#include "actioneditorimpl.h"
#include <qaction.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qpushbutton.h>

ActionEditor::ActionEditor( QWidget* parent,  const char* name, WFlags fl )
    : ActionEditorBase( parent, name, fl ), currentAction( 0 )
{
	enableAll( FALSE );
}

void ActionEditor::closeEvent( QCloseEvent *e )
{
    emit hidden();
    e->accept();
}

void ActionEditor::accelChanged( const QString & )
{
}

void ActionEditor::connectionsClicked()
{
}

void ActionEditor::currentActionChanged( QListViewItem *i )
{
    if ( !i ) {
	enableAll( FALSE );
	return;
    }
    enableAll( TRUE );
}

void ActionEditor::deleteAction()
{
}

void ActionEditor::enabledChanged( bool )
{
}

void ActionEditor::menuTextChanged( const QString & )
{
}

void ActionEditor::nameChanged( const QString & )
{
}

void ActionEditor::newAction()
{
}

void ActionEditor::onChanged( bool )
{
}

void ActionEditor::statusTipChanged( const QString & )
{
}

void ActionEditor::textChanged( const QString & )
{
}

void ActionEditor::toggleChanged( bool )
{
}

void ActionEditor::toolTipChanged( const QString & )
{
}

void ActionEditor::whatsThisChanged( const QString & )
{
}

void ActionEditor::chooseIcon()
{
}

void ActionEditor::enableAll( bool enable )
{
    editAccel->setEnabled( enable );
    editStatusTip->setEnabled( enable );
    editText->setEnabled( enable );
    editName->setEnabled( enable );
    labelIcon->setEnabled( enable );
    buttonIcon->setEnabled( enable );
    editToolTip->setEnabled( enable );
    editMenuText->setEnabled( enable );
    editWhatsThis->setEnabled( enable );
    listActions->setEnabled( enable );
    checkToggle->setEnabled( enable );
    checkOn->setEnabled( enable );
    checkEnabled->setEnabled( enable );
    buttonConnections->setEnabled( enable );
}
