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

void FindDialog::init()
{
    editor = 0;
    formWindow = 0;
}

void FindDialog::destroy()
{
    if ( editor )
	editor->release();
}

void FindDialog::doFind()
{
    if ( !editor )
	return;
    
    if ( !editor->find( comboFind->currentText(), checkCase->isChecked(),
	checkWords->isChecked(), radioForward->isChecked(), !checkBegin->isChecked() ) )
	checkBegin->setChecked( TRUE );
    else
	checkBegin->setChecked( FALSE );
    
}

void FindDialog::setEditor( EditorInterface * e, QObject * fw )
{
    if ( fw != formWindow )
	checkBegin->setChecked( TRUE );
    formWindow = fw;
    if ( editor )
	editor->release();
    editor = e;
    editor->addRef();
}
