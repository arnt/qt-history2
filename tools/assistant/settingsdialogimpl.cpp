/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Assistant.
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

#include <qcolordialog.h>
#include <qtoolbutton.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qdir.h>
#include <qptrstack.h>
#include <qsettings.h>
#include <qfileinfo.h>
#include <qtimer.h>
#include <qapplication.h>

#include "settingsdialogimpl.h"
#include "docuparser.h"

#define RTTI 3973

CheckListItem::CheckListItem( CheckListItem *parent, const QString &text,
    const QString &fullcat )
    : QCheckListItem(  parent, text, QCheckListItem::CheckBox ),
    fullCategory( fullcat )
{

}

CheckListItem::CheckListItem( QListView *parent, const QString &text,
    const QString &fullcat )
    : QCheckListItem(  parent, text, QCheckListItem::CheckBox ),
    fullCategory( fullcat )
{

}

int CheckListItem::rtti() const
{
    return RTTI;
}

CheckListItem* CheckListItem::getCurrentItem( QListView *parent )
{
    QListViewItem *i = parent->currentItem();
    if ( i->rtti() == RTTI )
	return (CheckListItem*)i;
    return 0;
}

CheckListItem* CheckListItem::getCheckItem( QListViewItem *i )
{
    if ( i->rtti() == RTTI )
	return (CheckListItem*)i;
    return 0;
}

void CheckListItem::stateChange( bool state )
{
    QPtrStack<stateListItem> stackState;

    QListViewItemIterator it( listView() );
    bool root = FALSE;
    bool disableNext = FALSE;
    int depth = -1;

    while( it.current() ) {
	it.current()->setEnabled( TRUE );
	if ( disableNext && it.current()->depth() > depth ) {
	    it.current()->setEnabled( FALSE );
	}
	if ( disableNext && it.current()->depth() <= depth ) {
	    disableNext = FALSE;
	    depth = -1;
	}
	if ( getCheckItem( it.current() )->isOn() && !disableNext ) {
	    depth = it.current()->depth();
	    disableNext = TRUE;
	}
	++it;
    }
}

QString CheckListItem::getFullCategory()
{
    return fullCategory;
}

SettingsDialog::SettingsDialog( QWidget *parent, const char* name )
    : SettingsDialogBase( parent, name )
{
    init();
}

void SettingsDialog::init()
{
    QSettings settings;
    changed = FALSE;
    selectionChanged = FALSE;

    QString b = settings.readEntry( "/Qt Assistant/3.1/Webbrowser/", "" );
    browserApp->setText( b );

    docuFileBox->clear();
    docuFileList = settings.readListEntry( "/Qt Assistant/3.1/AdditionalDocFiles/" );
    docuTitleList = settings.readListEntry( "/Qt Assistant/3.1/AdditionalDocTitles/" );
    QStringList::iterator it = docuTitleList.begin();
    for ( ; it != docuTitleList.end(); ++it )
	docuFileBox->insertItem( *it );

    catListAvail = settings.readListEntry( "/Qt Assistant/3.1/CategoriesAvailable/" );
    catListSel = settings.readListEntry( "/Qt Assistant/3.1/CategoriesSelected/" );

    insertCategories();
}

void SettingsDialog::insertCategories()
{
    if ( catListAvail.isEmpty() )
	catListSel << "all";

    catListView->clear();
    catItemList.clear();

    allItem = new CheckListItem( catListView, tr( "all" ), "all" );
    checkItem( allItem );
    catItemList.append( allItem );

    makeCategoryList();

    QPtrList<listItem> itemList;
    QStringList::iterator it = catListAvail.begin();
    for ( ; it != catListAvail.end(); ++it ) {
	QString str( *it );
	int pos = str.findRev( '/' );
	QString sn = str.right( str.length() - pos - 1 );
	listItem *li = new listItem( str, sn, str.contains( '/' ) );
	itemList.append( li );
    }

    QPtrListIterator<listItem> pit(itemList);
    listItem *item;
    QPtrStack<CheckListItem> stack;
    stack.clear();
    CheckListItem *listItem;
    int depth = 0;
    bool root = FALSE;

    while ( ( item = pit.current() ) != 0 ) {
	++pit;
	if( item->d == 0 ){
	    listItem = new CheckListItem( allItem, item->sname, item->lname );
	    checkItem( listItem );
	    catItemList.append( listItem );
	    stack.push( listItem );
	    depth = 1;
	    root = TRUE;
	}
	else{
	    if( (item->d > depth) && (root) ){
		depth = item->d;
		stack.push( listItem );
	    }
	    if( item->d == depth ){
	        listItem = new CheckListItem( stack.top(), item->sname, item->lname );
		checkItem( listItem );
		catItemList.append( listItem );
	    }
	    else if( item->d < depth ){
		stack.pop();
		depth--;
		--pit;
	    }
	}
    }
    allItem->setOpen( TRUE );
    if ( !itemList.isEmpty() )
	listItem->stateChange( FALSE );
}

void SettingsDialog::checkItem( CheckListItem *item )
{
    if ( catListSel.find( item->getFullCategory() ) != catListSel.end() )
	item->setOn( TRUE );
}

void SettingsDialog::makeCategoryList( void )
{
    catListAvail.sort();
    QStringList::iterator i = catListAvail.begin();

    for ( ; i != catListAvail.end(); ++i ) {
	QString str( *i );
	int pos = str.findRev( '/' );
        str.remove( pos,  str.length() - pos );
        bool found = FALSE;
        if ( catListAvail.find( str ) != catListAvail.end() )
	    found = TRUE;
	if ( !found ) {
	    i = catListAvail.insert( i, str );
	    i = catListAvail.begin();
	}
    }
    catListAvail.sort();
}

void SettingsDialog::selectColor()
{
    QColor c = QColorDialog::getColor( colorButton->paletteBackgroundColor(), this );
    colorButton->setPaletteBackgroundColor( c );
}

void SettingsDialog::addDocuFile()
{
    QFileDialog *fd = new QFileDialog ( QDir::homeDirPath(), "xml Files (*.xml)", this );

    if ( fd->exec() == QDialog::Accepted ) {
	QString file = fd->selectedFile();
	QFileInfo fi( file );
	if ( !fi.isReadable() ) {
	    QMessageBox::warning( this, tr( "Qt Assistant" ),
			tr( "File " + file + " is not readable!" ) );
	    return;
	}
	QFile f( file );
	DocuParser handler;
	QXmlInputSource source( f );
        QXmlSimpleReader reader;
	reader.setContentHandler( &handler );
	reader.setErrorHandler( &handler );
	setCursor( waitCursor );
	qApp->processEvents();
	bool ok = reader.parse( source );
	setCursor( arrowCursor );
	f.close();
	if ( !ok ) {
	    QMessageBox::warning( this, tr( "Qt Assistant" ),
			tr( "Error in file %1:\n%2" ).arg( file ).arg( handler.errorProtocol() ) );
	    return;
	}
	QString title = handler.getDocumentationTitle();
	if ( title.isEmpty() )
	    title = file;
	docuFileBox->insertItem( title );
	docuTitleList << title;
	docuFileList << file;
	changed = TRUE;
	QString cat = handler.getCategory();
	if ( cat.isEmpty() )
	    return;
	if ( catListAvail.find( cat ) == catListAvail.end() )
	    catListAvail << cat;
	if ( catListSel.find( cat ) == catListSel.end() ) {
	    catListSel << cat;
	    selectionChanged = TRUE;
	}
	insertCategories();
    }
}

void SettingsDialog::removeDocuFile()
{
    QStringList::iterator it = docuTitleList.begin();
    QStringList::iterator it1 = docuFileList.begin();

    while ( it != docuTitleList.end() && it1 != docuFileList.end() ) {
	if ( *it == docuFileBox->currentText() ) {
	    docuTitleList.remove( it );
	    docuFileList.remove( it1 );
	    changed = TRUE;
	    break;
	}
	++it;
	++it1;
    }
    docuFileBox->removeItem( docuFileBox->currentItem() );
}

void SettingsDialog::addCategory()
{
    QString cat = catName->text();
    catName->clear();
    if( cat.isEmpty() || cat.find( '/' ) > -1  )
	return;

    CheckListItem *item = allItem;
    item = item->getCurrentItem( catListView );

    if ( item == 0 )
	return;

    QString fullcat = item->getFullCategory() + "/" + cat;
    if ( catListAvail.find( fullcat ) == catListAvail.end() ) {
	catListAvail << fullcat;
	CheckListItem *cl = new CheckListItem( item, cat, fullcat );
	item->stateChange( item->isOn() );
	catItemList.append( cl );
    }
}

void SettingsDialog::deleteCategory()
{
    CheckListItem *item = allItem;
    item = item->getCurrentItem( catListView );

    if ( item->getFullCategory().lower() == "all" ) {
	QMessageBox::warning( this, tr( "Qt Assistant" ),
	    tr( "This item can not be deleted!" ) );
	return;
    }

    if ( item->firstChild() != 0 ) {
	QMessageBox::warning( this, tr( "Qt Assistant" ),
	    tr( "This branch is not empty!\nFirstly, all contents have to be removed." ) );
	return;
    }

    QStringList::iterator it = catListAvail.begin();
    bool deleted = FALSE;
    for ( ; it != catListAvail.end(); ++it ) {
	if ( ( (*it).lower() == item->getFullCategory().lower() ) && !deleted ) {
	    delete item;
	    catListSel.remove( *it );
	    it = catListAvail.remove( it );
	    if ( it != catListAvail.begin() )
		it--;
	    changed = TRUE;
	    deleted = TRUE;
	}
    }
}

void SettingsDialog::browseWebApp()
{
    QFileDialog *fd = new QFileDialog( this );
    fd->setMode( QFileDialog::AnyFile );
    fd->setDir( QDir::homeDirPath() );

    if ( fd->exec() == QDialog::Accepted ) {
	if ( !fd->selectedFile().isEmpty() )
	    browserApp->setText( fd->selectedFile() );
    }
}

void SettingsDialog::accept()
{
    QSettings *settings = new QSettings();
    settings->writeEntry( "/Qt Assistant/3.1/AdditionalDocFiles/", docuFileList );
    settings->writeEntry( "/Qt Assistant/3.1/AdditionalDocTitles/", docuTitleList );
    settings->writeEntry( "/Qt Assistant/3.1/CategoriesAvailable/", catListAvail );

    settings->writeEntry( "/Qt Assistant/3.1/Webbrowser/", browserApp->text() );

    hide();

    bool changedCategory = FALSE;
    catListSel.sort();
    QStringList newCatListSel = getCheckedItemList();
    if ( catListSel != newCatListSel || selectionChanged ) {
	catListSel = newCatListSel;
	settings->writeEntry( "/Qt Assistant/3.1/CategoriesSelected/", catListSel );
	changedCategory = TRUE;
    }

    delete settings;
    settings = 0;

    if ( changed ||  changedCategory ) {
	changed = FALSE;
	emit docuFilesChanged();
    }
    done( Accepted );
}

void SettingsDialog::reject()
{
    init();
    done( Rejected );
}

QStringList SettingsDialog::getCheckedItemList()
{
    QStringList list;
    QPtrStack<stateListItem> stackState;
    CheckListItem *ci = allItem;

    QListViewItemIterator it( catListView );
    bool root = FALSE;
    bool isChecked = FALSE;
    int depth = -1;
    while( it.current() ){
	if( it.current()->depth() == 0 ){
	    isChecked = ci->getCheckItem( it.current() )->isOn();
	    stackState.push( new stateListItem( it.current(), isChecked ) );
	    if ( isChecked )
		list << ci->getCheckItem( it.current() )->getFullCategory();
	    depth = it.current()->depth();
	    root = TRUE;
	    isChecked = FALSE;
	}
	else{
	    if( (it.current()->depth() > depth) && (root) ){
		depth = it.current()->depth();
		isChecked = FALSE;
		isChecked = ci->getCheckItem( it.current() )->isOn() || stackState.top()->isChecked;
		stackState.push( new stateListItem( it.current(), isChecked ) );
	    }
	    if( it.current()->depth() == depth ){
		stackState.pop();
		isChecked = FALSE;
		isChecked = ci->getCheckItem( it.current() )->isOn() || stackState.top()->isChecked;
		stackState.push( new stateListItem( it.current(), isChecked ) );
		if ( isChecked )
		    list << ci->getCheckItem( it.current() )->getFullCategory();
	    }
	    else if( it.current()->depth() < depth ){
		stackState.pop();
		depth--;
		it--;
	    }
	}
	++it;
    }
    list.sort();
    return list;
}

QStringList SettingsDialog::documentationList() const
{
    return docuFileList;
}

QStringList SettingsDialog::selCategoriesList() const
{
    return catListSel;
}
