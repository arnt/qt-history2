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

#include "editfunctionsimpl.h"
#include "formwindow.h"
#include "metadatabase.h"
#include "asciivalidator.h"
#include "mainwindow.h"
#include "hierarchyview.h"
#include "project.h"

#include <qlistview.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qstrlist.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qradiobutton.h>

EditFunctions::EditFunctions( QWidget *parent, FormWindow *fw, bool justSlots )
    : EditFunctionsBase( parent, 0, TRUE ), formWindow( fw )
{
    LanguageInterface *iface = MetaDataBase::languageInterface( fw->project()->language() );
    if ( iface && !iface->supports( LanguageInterface::ReturnType ) ) {
	functionListView->removeColumn( 3 );
	editType->hide();
	labelType->hide();
    }

    connect( helpButton, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );
    QValueList<MetaDataBase::Function> functionList = MetaDataBase::functionList( fw );
    for ( QValueList<MetaDataBase::Function>::Iterator it = functionList.begin(); it != functionList.end(); ++it ) {
	QListViewItem *i = new QListViewItem( functionListView );
	oldFunctionNames.insert( i, QString( (*it).function ) );
	i->setPixmap( 0, PixmapChooser::loadPixmap( "editslots.xpm" ) );
	i->setText( 0, (*it).function );
	i->setText( 1, (*it).returnType );
	i->setText( 2, (*it).specifier );
	i->setText( 3, (*it).access  );
	i->setText( 4, (*it).type );
		
	if ( (*it).type == "slot" ) {
	    if ( MetaDataBase::isSlotUsed( formWindow, MetaDataBase::normalizeFunction( (*it).function ).latin1() ) )
		i->setText( 5, tr( "Yes" ) );
	    else
		i->setText( 5, tr( "No" ) );
	} else {
	    i->setText( 5, "---" );
	}	
    }
    
    boxProperties->setEnabled( FALSE );
    functionName->setValidator( new AsciiValidator( TRUE, functionName ) );

    if ( functionListView->firstChild() )
	functionListView->setCurrentItem( functionListView->firstChild() );

    showOnlySlots->setChecked( justSlots );
}

void EditFunctions::okClicked()
{    
    QValueList<MetaDataBase::Function> functionList = MetaDataBase::functionList( formWindow );
    MacroCommand *rmFunc = 0, *addFunc = 0;
    QString n = tr( "Add/Remove functions of '%1'" ).arg( formWindow->name() );
    QValueList<MetaDataBase::Function>::Iterator fit;
    if ( !functionList.isEmpty() ) {
	QPtrList<Command> commands;
	for ( fit = functionList.begin(); fit != functionList.end(); ++fit ) {
	    commands.append( new RemoveFunctionCommand( tr( "Remove function" ),
						    formWindow, (*fit).function, (*fit).specifier, (*fit).access,
						    (*fit).type, formWindow->project()->language(), (*fit).returnType ) );
	}
	rmFunc = new MacroCommand( tr( "Remove functions" ), formWindow, commands );
    }

    bool invalidFunctions = FALSE;
    QPtrList<QListViewItem> invalidItems;
    if ( functionListView->firstChild() ) {
	QPtrList<Command> commands;
	QListViewItemIterator it( functionListView );
	QStrList lst;
	for ( ; it.current(); ++it ) {
	    MetaDataBase::Function function;
	    function.function = it.current()->text( 0 );
	    function.returnType = it.current()->text( 1 );
	    function.specifier = it.current()->text( 2 );
	    function.access = it.current()->text( 3 );
	    function.type = it.current()->text(4 );
	    function.language = formWindow->project()->language();
	    if ( function.returnType.isEmpty() )
		function.returnType = "void";
	    QString s = function.function;
	    s = s.simplifyWhiteSpace();
	    bool startNum = s[ 0 ] >= '0' && s[ 0 ] <= '9';
	    bool noParens = s.contains( '(' ) != 1 || s.contains( ')' ) != 1;
	    bool illegalSpace = s.find( ' ' ) != -1 && s.find( ' ' ) < s.find( '(' );

	    if ( startNum || noParens || illegalSpace || lst.find( function.function ) != -1 ) {
		invalidFunctions = TRUE;
		invalidItems.append( it.current() );
		continue;
	    }
	    commands.append( new AddFunctionCommand( tr( "Add function" ),  
						 formWindow, function.function, function.specifier, function.access,
						 function.type, formWindow->project()->language(),
						 function.returnType ) );
	    QMap<QListViewItem*, QString>::Iterator sit = oldFunctionNames.find( it.current() );
	    if ( sit != oldFunctionNames.end() ) {
		if ( *sit != it.current()->text( 0 ) )
		    MetaDataBase::functionNameChanged( formWindow, *sit, it.current()->text( 0 ) );
	    }
	    lst.append( function.function );
	}
	if ( !commands.isEmpty() )
	    addFunc = new MacroCommand( tr( "Add functions" ), formWindow, commands );
    }
    if ( invalidFunctions ) {
	if ( QMessageBox::information( this, tr( "Edit Functions" ),
					     tr( "Some syntactically incorrect functions have been defined.\n"
						 "Remove these functions?" ),
				       tr( "&Yes" ), tr( "&No" ) ) == 0 ) {
	    QListViewItemIterator it( functionListView );
	    QListViewItem *i;
	    while ( (i = it.current() ) ) {
		++it;
		if ( invalidItems.findRef( i ) != -1 )
		    delete i;
	    }
	    if ( functionListView->firstChild() ) {
		functionListView->setCurrentItem( functionListView->firstChild() );
		functionListView->setSelected( functionListView->firstChild(), TRUE );
	    }
	}
	formWindow->mainWindow()->objectHierarchy()->updateFormDefinitionView();
	return;
    }
    for ( QStringList::Iterator rsit = removedFunctions.begin(); rsit != removedFunctions.end(); ++rsit )
	removeFunctionFromCode( *rsit, formWindow ); 

    if ( rmFunc || addFunc ) {
	QPtrList<Command> commands;
	if ( rmFunc )
	    commands.append( rmFunc );
	if ( addFunc )
	    commands.append( addFunc );
	MacroCommand *cmd = new MacroCommand( n, formWindow, commands );
	formWindow->commandHistory()->addCommand( cmd );
	cmd->execute();
    }

    formWindow->mainWindow()->objectHierarchy()->updateFormDefinitionView();
    accept();
}

void EditFunctions::functionAdd( const QString &access, const QString &type )
{    
    QListViewItem *i = new QListViewItem( functionListView );
    i->setPixmap( 0, PixmapChooser::loadPixmap( "editslots.xpm" ) ); 
    i->setText( 0, "newFunction()" );
    i->setText( 1, "void" );
    i->setText( 2, "virtual" );
    if ( access.isEmpty() )
	i->setText( 3, "public" );
    else
	i->setText( 3, access );
    
    if( type.isEmpty() )
	i->setText( 4, "function" );
    else
	i->setText( 4, type );
    
    if ( i->text( 4 ) == "slot" ) {
	i->setText( 0, "newSlot()" );
	if ( MetaDataBase::isSlotUsed( formWindow, "newSlot()" ) )
	    i->setText( 5, tr( "Yes" ) );
	else    
	    i->setText( 5, tr( "No" ) );    
    } else {
	i->setText( 5, "---" );
    }
    functionListView->setCurrentItem( i );
    functionListView->setSelected( i, TRUE );
    functionName->setFocus();
    functionName->selectAll();    
}

void EditFunctions::functionRemove()
{    
    if ( !functionListView->currentItem() )
	return;

    functionListView->blockSignals( TRUE );
    removedFunctions << MetaDataBase::normalizeFunction( functionListView->currentItem()->text( 0 ) );
    delete functionListView->currentItem();
    if ( functionListView->currentItem() )
	functionListView->setSelected( functionListView->currentItem(), TRUE );
    functionListView->blockSignals( FALSE );
    currentItemChanged( functionListView->currentItem() );
}

void EditFunctions::currentItemChanged( QListViewItem *i )
{    
    functionName->blockSignals( TRUE );
    functionName->setText( "" );
    functionAccess->setCurrentItem( 0 );
    functionName->blockSignals( FALSE );

    if ( !i ) {
	boxProperties->setEnabled( FALSE );
	return;
    }

    functionName->blockSignals( TRUE );
    functionName->setText( i->text( 0 ) );
    editType->setText( i->text( 1 ) );
    QString specifier = i->text( 2 );
    QString access = i->text( 3 );
    QString type = i->text( 4 );
    if ( specifier == "pure virtual" )
	functionSpecifier->setCurrentItem( 2 );
    else if ( specifier == "non virtual" )
	functionSpecifier->setCurrentItem( 0 );
    else
	functionSpecifier->setCurrentItem( 1 );
    if ( access == "private" )
	functionAccess->setCurrentItem( 2 );
    else if ( access == "protected" )
	functionAccess->setCurrentItem( 1 );
    else
	functionAccess->setCurrentItem( 0 );
    if ( type == "slot" )
	functionType->setCurrentItem( 0 );
    else
	functionType->setCurrentItem( 1 );
	
    functionName->blockSignals( FALSE );
    boxProperties->setEnabled( TRUE );   
}

void EditFunctions::currentTextChanged( const QString &txt )
{
    if ( !functionListView->currentItem() )
	return;

    functionListView->currentItem()->setText( 0, txt );
    
    if ( functionListView->currentItem()->text( 4 ) == "slot" ) {
	if ( MetaDataBase::isSlotUsed( formWindow, MetaDataBase::normalizeFunction( txt.latin1() ).latin1() ) )
	    functionListView->currentItem()->setText( 5, tr( "Yes" ) );
	else
	    functionListView->currentItem()->setText( 5, tr( "No" ) );    
    } else {
	functionListView->currentItem()->setText( 5, "---" );
    }
}

void EditFunctions::currentSpecifierChanged( const QString& s )
{
    if ( !functionListView->currentItem() )
	return;

    functionListView->currentItem()->setText( 2, s );
}

void EditFunctions::currentAccessChanged( const QString& a )
{
    if ( !functionListView->currentItem() )
	return;
    functionListView->currentItem()->setText( 3, a );
}


void EditFunctions::currentReturnTypeChanged( const QString &type )
{
    if ( !functionListView->currentItem() )
	return;

    functionListView->currentItem()->setText( 1, type );
}

void EditFunctions::currentTypeChanged( const QString &type )
{
    if ( !functionListView->currentItem() )
	return;
    functionListView->currentItem()->setText( 4, type );
}

void EditFunctions::removeFunctionFromCode( const QString &function, FormWindow *formWindow )
{
    formWindow->formFile()->checkTimeStamp();
    QString code = formWindow->formFile()->code();
    if ( code.isEmpty() || !formWindow->formFile()->hasFormCode() )
	return;
    LanguageInterface *iface = MetaDataBase::languageInterface( formWindow->project()->language() );
    if ( !iface )
	return;
    QValueList<LanguageInterface::Function> functions;
    iface->functions( code, &functions );
    QString fu = MetaDataBase::normalizeFunction( function );
    for ( QValueList<LanguageInterface::Function>::Iterator fit = functions.begin(); fit != functions.end(); ++fit ) {
	if ( MetaDataBase::normalizeFunction( (*fit).name ) == fu ) {
	    int line = 0;
	    int start = 0;
	    while ( line < (*fit).start - 1 ) {
		start = code.find( '\n', start );
		if ( start == -1 )
		    return;
		start++;
		line++;
	    }
	    if ( start == -1 )
		return;
	    int end = start;
	    while ( line < (*fit).end + 1 ) {
		end = code.find( '\n', end );
		if ( end == -1 ) {
		    if ( line <= (*fit).end )
			end = code.length() - 1;
		    else
			return;
		}
		end++;
		line++;
	    }
	    if ( end < start )
		return;
	    code.remove( start, end - start );
	    formWindow->formFile()->setCode( code );
	}
    }
}

void EditFunctions::setCurrentFunction( const QString &function )
{    
    QListViewItemIterator it( functionListView );
    while ( it.current() ) {
	if ( MetaDataBase::normalizeFunction( it.current()->text( 0 ) ) == function ) {
	    functionListView->setCurrentItem( it.current() );
	    functionListView->setSelected( it.current(), TRUE );
	    currentItemChanged( it.current() );
	    return;
	}
	++it;
    }
}

void EditFunctions::displaySlots( bool justSlots )
{
    QValueList<MetaDataBase::Function> functList;    
    if ( justSlots ) 
        functList = MetaDataBase::slotList( formWindow );
    else
	functList = MetaDataBase::functionList( formWindow );
    
    functionListView->clear();
    for ( QValueList<MetaDataBase::Function>::Iterator it = functList.begin(); it != functList.end(); ++it ) {
	QListViewItem *i = new QListViewItem( functionListView );
	i->setPixmap( 0, PixmapChooser::loadPixmap( "editslots.xpm" ) );
	i->setText( 0, (*it).function );
	i->setText( 1, (*it).returnType );
	i->setText( 2, (*it).specifier );
	i->setText( 3, (*it).access  );
	i->setText( 4, (*it).type );
		
	if ( (*it).type == "slot" ) {
	    if ( MetaDataBase::isSlotUsed( formWindow, MetaDataBase::normalizeFunction( (*it).function ).latin1() ) )
		i->setText( 5, tr( "Yes" ) );
	    else
		i->setText( 5, tr( "No" ) );
	} else {
	    i->setText( 5, "---" );
	}	
    }    
}
