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

