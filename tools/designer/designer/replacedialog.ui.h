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

void ReplaceDialog::init()
{
    editor = 0;
    formWindow = 0;
}

void ReplaceDialog::destroy()
{
    if ( editor )
	editor->release();
    editor = 0;
    formWindow = 0;
}

void ReplaceDialog::doReplace()
{
    if ( !editor ) 
	return; 
     
    if ( !editor->replace( comboFind->currentText(), comboReplace->currentText(), checkCase->isChecked(), 
	checkWords->isChecked(), radioForward->isChecked(), !checkBegin->isChecked(), false ) ) 
	checkBegin->setChecked( true ); 
    else 
	checkBegin->setChecked( false ); 
}

void ReplaceDialog::doReplaceAll()
{
    if ( !editor )  
	return;  
      
    if ( !editor->replace( comboFind->currentText(), comboReplace->currentText(), checkCase->isChecked(),  
	checkWords->isChecked(), radioForward->isChecked(), !checkBegin->isChecked(), true ) )  
	checkBegin->setChecked( true );  
    else  
	checkBegin->setChecked( false );  
}

void ReplaceDialog::setEditor( EditorInterface * e, QObject * fw )
{
    if ( fw != formWindow )
	checkBegin->setChecked( true );
    formWindow = fw;
    if ( editor )
	editor->release();
    editor = e;
    editor->addRef();
}
