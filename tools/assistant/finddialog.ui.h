void FindDialog::init()
{
    browser = 0;
    fromBegin = TRUE;
}

void FindDialog::destroy()
{
}

void FindDialog::doFind()
{
    if ( !browser )
	return;
    
    int dummy = 0;
    if ( !fromBegin )
	fromBegin = !browser->find( comboFind->currentText(), checkCase->isChecked(),
			checkWords->isChecked(), radioForward->isChecked() );    
    else
	fromBegin = !browser->find( comboFind->currentText(), checkCase->isChecked(), 
			checkWords->isChecked(), radioForward->isChecked(), &dummy, &dummy );    
}

void FindDialog::setBrowser( QTextBrowser * b )
{
    browser = b;
    fromBegin = TRUE;
}

