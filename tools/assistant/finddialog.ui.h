/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt Assistant.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qapplication.h>

void FindDialog::init()
{
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
    QTextBrowser *browser = (QTextBrowser*) mainWindow()->browsers()->currentBrowser();
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



MainWindow* FindDialog::mainWindow()
{
    return (MainWindow*) parent();
}
