#include <qapplication.h>

void FindDialog::init()
{
    browser = 0;
    fromBegin = TRUE;
    firstRun = TRUE;
    onceFound = FALSE;
    findExpr = "";
    sb = new QStatusBar( this );
    FindDialogLayout->addWidget( sb );
    sb->message( tr( "Enter the text you are looking for." ) );
}

void FindDialog::destroy()
{
}

void FindDialog::doFind()
{
    if ( !browser )
	return;
    sb->clear();
    if ( comboFind->currentText() != findExpr ) 
	onceFound = FALSE;
    findExpr = comboFind->currentText();
        
    int dummy = radioForward->isChecked() ? 0 : INT_MAX;
    if ( !fromBegin )
	fromBegin = !browser->find( findExpr, checkCase->isChecked(),
			checkWords->isChecked(), radioForward->isChecked() );
    else
	fromBegin = !browser->find( findExpr, checkCase->isChecked(),
			checkWords->isChecked(), radioForward->isChecked(), &dummy, &dummy );
    if ( fromBegin ) {
	QApplication::beep();
	if ( onceFound ) {
	    if ( radioForward->isChecked() )
		sb->message( tr( "Search reached end of the document" ) );
	    else
		sb->message( tr( "Search reached start of the document" ) );
	} else
	    sb->message( tr( "Text not found" ) );  
    } else
	onceFound = TRUE;
}

void FindDialog::setBrowser( QTextBrowser * b )
{
    browser = b;
    fromBegin = TRUE;
}
