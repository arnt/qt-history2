#include <qapplication.h>

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

    int dummy = radioForward->isChecked() ? 0 : INT_MAX;
    if ( !fromBegin )
	fromBegin = !browser->find( comboFind->currentText(), checkCase->isChecked(),
			checkWords->isChecked(), radioForward->isChecked() );
    else
	fromBegin = !browser->find( comboFind->currentText(), checkCase->isChecked(),
			checkWords->isChecked(), radioForward->isChecked(), &dummy, &dummy );
    if ( fromBegin )
	QMessageBox::information( this, tr( "Find Text" ),
				  tr( "Can not find requested text in this document." ) );	
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
