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

#include <qcolordialog.h>
#include <qtoolbutton.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qdir.h>
#include <qsettings.h>

#include "settingsdialogimpl.h"

SettingsDialog::SettingsDialog( QWidget *parent, const char* name )
    : SettingsDialogBase( parent, name )
{
    init();
}

void SettingsDialog::init()
{
    QSettings settings;
    changed = FALSE;
    
    QString b = settings.readEntry( "/Qt Assistant/3.1/Webbrowser/", "" );
    browserApp->setText( b );
    
    docuPathBox->clear();
    catitems.clear();
    pathlist = settings.readListEntry( "/Qt Assistant/additionalDocu/Path/" );
    QStringList::iterator i = pathlist.begin();
    QStringList::iterator j;
    QCheckListItem *cl;
    catListView->clear();
    for ( ; i != pathlist.end(); ++i )
	docuPathBox->insertItem( *i );
    
    catlistavail = settings.readListEntry( "/Qt Assistant/categories/available/" );
    catlistsel = settings.readListEntry( "/Qt Assistant/categories/selected/" );
    bool nocatavail = catlistavail.isEmpty();    
    j = catlistavail.find( "all" );
    if ( catlistavail.find( "all" ) == catlistavail.end() ) 
	catlistavail << "all";
    if ( nocatavail )
	catlistsel << "all";    
    for ( i = catlistavail.begin(); i != catlistavail.end(); ++i ) {
	cl = new QCheckListItem( catListView, *i, QCheckListItem::CheckBox );
	catitems.append( cl );
	j = catlistsel.find( *i );
	if ( j != catlistsel.end() )
	    cl->setOn( TRUE ); 	
	else
	    cl->setOn( FALSE );
    }
}

void SettingsDialog::selectColor()
{
    QColor c = QColorDialog::getColor( colorButton->paletteBackgroundColor(), this );
    colorButton->setPaletteBackgroundColor( c );
}

void SettingsDialog::addPath()
{
    QFileDialog *fd = new QFileDialog( this );
    fd->setMode( QFileDialog::DirectoryOnly );    
    fd->setDir( QDir::homeDirPath() );
    
    if ( fd->exec() == QDialog::Accepted ) {
	if ( newFilesExist( fd->selectedFile() )) {
	    docuPathBox->insertItem( fd->selectedFile() );
	    pathlist << fd->selectedFile();
	    changed = TRUE;
	}
    }
}

void SettingsDialog::deletePath()
{
    QStringList::iterator it = pathlist.begin();
    bool deleted = FALSE;
    for ( ; it != pathlist.end(); ++it ) {
	if ( (*it == docuPathBox->currentText()) && !deleted ) {
	    docuPathBox->removeItem( docuPathBox->currentItem() );	    
	    it = pathlist.remove( it );
	    if ( it != pathlist.begin() )
		it--;
	    changed = TRUE;
	    deleted = TRUE;
	}
    }
}

void SettingsDialog::addCategory()
{
    QString cat = catName->text();
    catName->clear();
    if( cat.isEmpty() )
	return;
    catlistavail << cat;
    QCheckListItem *cl = new QCheckListItem( catListView, cat, QCheckListItem::CheckBox );
    catitems.append( cl );
}

void SettingsDialog::deleteCategory()
{
    QStringList::iterator it = catlistavail.begin();
    bool deleted = FALSE;
    for ( ; it != catlistavail.end(); ++it ) {
	if ( ( *it == catListView->currentItem()->text( 0 ) ) && !deleted ) {
	    catListView->removeItem( catListView->currentItem() );
	    catlistsel.remove( *it );
	    it = catlistavail.remove( it );
	    if ( it != catlistavail.begin() )
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
    settings->writeEntry( "/Qt Assistant/additionalDocu/Path/", pathlist );    
    settings->writeEntry( "/Qt Assistant/categories/available/", catlistavail );
    
    settings->writeEntry( "/Qt Assistant/3.1/Webbrowser/", browserApp->text() );
    QStringList old = catlistsel;
    catlistsel.clear();
    QPtrListIterator<QCheckListItem> it( catitems );
    QCheckListItem *item;
    while ( (item = it.current()) != 0 ) {
	++it;
	if ( item->isOn() )
	    catlistsel << item->text();
    }	
    settings->writeEntry( "/Qt Assistant/categories/selected/", catlistsel );
    delete settings;
    settings = 0;
    hide();
    if ( old != catlistsel ) 
	emit changedCategory();    
    
    if ( changed ) {
	changed = FALSE;
	emit changedPath();
    }
    done( Accepted );
}

void SettingsDialog::reject()
{    
    init();
    done( Rejected );
}

bool SettingsDialog::newFilesExist( QString dir )
{
    if ( !QFile::exists( dir + "contents.xml" )) {
        QMessageBox::critical( this, "Error", 
		     "File " + dir + "content.xml does not exist!" );
        return FALSE;
    }
    if ( !QFile::exists( dir + "index.xml" )) {
        QMessageBox::critical( this, "Error", 
		     "File " + dir + "index.xml does not exist!" );
        return FALSE;
    }
    return TRUE;
}

