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
	checkWords->isChecked(), radioForward->isChecked(), !checkBegin->isChecked(), FALSE ) ) 
	checkBegin->setChecked( TRUE ); 
    else 
	checkBegin->setChecked( FALSE ); 
}

void ReplaceDialog::doReplaceAll()
{
    if ( !editor )  
	return;  
      
    if ( !editor->replace( comboFind->currentText(), comboReplace->currentText(), checkCase->isChecked(),  
	checkWords->isChecked(), radioForward->isChecked(), !checkBegin->isChecked(), TRUE ) )  
	checkBegin->setChecked( TRUE );  
    else  
	checkBegin->setChecked( FALSE );  
}

void ReplaceDialog::setEditor( EditorInterface * e, QObject * fw )
{
    if ( fw != formWindow )
	checkBegin->setChecked( TRUE );
    formWindow = fw;
    if ( editor )
	editor->release();
    editor = e;
    editor->addRef();
}

