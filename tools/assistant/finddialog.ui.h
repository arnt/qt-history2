void FindDialog::init()
{
    browser = 0;
    fromBegin = TRUE;
    firstRun = TRUE;
}

void FindDialog::destroy()
{
}

void FindDialog::doFind()
{
    if ( !browser )
	return;
   
    int para = 0;
    int index = 0;
    
    if ( !radioForward->isChecked() ) {
	para = browser->paragraphs() - 1;
	index = browser->paragraphLength( para );
	if ( index == -1 )
	    index = 0;
    }
    
    if ( !fromBegin ) 
	fromBegin = !browser->find( comboFind->currentText(), checkCase->isChecked(),
			checkWords->isChecked(), radioForward->isChecked() );    
    else 
	fromBegin = !browser->find( comboFind->currentText(), checkCase->isChecked(), 
			checkWords->isChecked(), radioForward->isChecked(), &para, &index );    
    
    if ( fromBegin && firstRun ) {
	fromBegin = !browser->find( comboFind->currentText(), checkCase->isChecked(),
			checkWords->isChecked(), radioForward->isChecked(), &para, &index ); 
	if ( fromBegin )
	    QMessageBox::information( this, tr( "Find Text" ), 
				      tr( "Can not find requested text in this document." ) );	
    }
    if ( !fromBegin ) 
	firstRun = FALSE;
}

void FindDialog::setBrowser( QTextBrowser * b )
{
    browser = b;
    fromBegin = TRUE;
}

void FindDialog::sthChanged()
{
    firstRun = TRUE;
}
