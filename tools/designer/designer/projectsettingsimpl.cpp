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

#include "projectsettingsimpl.h"
#include "project.h"
#include "formwindow.h"
#include "metadatabase.h"
#include "mainwindow.h"

#include <qlineedit.h>
#include <qtextedit.h>
#include <qlistbox.h>
#include <qfiledialog.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qobjectlist.h>
#include <qheader.h>

/*
 *  Constructs a ProjectSettings which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
ProjectSettings::ProjectSettings( Project *pro, QWidget* parent,  const char* name, bool modal, WFlags fl )
    : ProjectSettingsBase( parent, name, modal, fl ), project( pro )
{
    editProjectName->setText( pro->projectName() );
    editProjectFile->setText( pro->fileName() );
    editProjectDescription->setText( pro->description() );

    QStringList lst = project->uiFiles();
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
	QListViewItem *item = new QListViewItem( listInterfaces, tr( "<unknown>" ), *it, 0 );
	QString className = project->formName( item->text( 1 ) );
	if ( !className.isEmpty() )
	    item->setText( 0, className );
    }

    QObjectList *l = parent->queryList( "FormWindow", 0, FALSE, TRUE );
    for ( QObject *o = l->first(); o; o = l->next() ) {
	if ( ( (FormWindow*)o )->project() != project )
	    continue;
	QListViewItemIterator it( listInterfaces );
	while ( it.current() ) {
	    if ( project->makeAbsolute( ( it.current() )->text( 1 ) ) ==
		 project->makeAbsolute( ( (FormWindow*)o )->fileName() ) ) {
		it.current()->setText( 0, o->name() );
		formMap.insert( it.current(), (FormWindow*)o );
	    }
	    ++it;
	}
    }


    listInterfaces->header()->setFullSize( TRUE );

    editDatabaseFile->setText( pro->databaseDescription() );

    comboLanguage->insertStringList( MetaDataBase::languages() );
    for ( int j = 0; j < (int)comboLanguage->count(); ++j ) {
	if ( project->language() == comboLanguage->text( j ) ) {
	    comboLanguage->setCurrentItem( j );
	    break;
	}
    }
}

/*
 *  Destroys the object and frees any allocated resources
 */
ProjectSettings::~ProjectSettings()
{
}

void ProjectSettings::chooseDatabaseFile()
{
    QString fn = QFileDialog::getSaveFileName( QString::null, tr( "Project Files (*.db);;All Files (*)" ), this );
    if ( fn.isEmpty() )
	return;
    editDatabaseFile->setText( fn );
}

void ProjectSettings::chooseProjectFile()
{
    QString fn = QFileDialog::getSaveFileName( QString::null, tr( "Project Files (*.pro);;All Files (*)" ), this );
    if ( fn.isEmpty() )
	return;
    editProjectFile->setText( fn );
}

void ProjectSettings::helpClicked()
{
}

void ProjectSettings::okClicked()
{
    // ### check for validity

    project->setProjectName( editProjectName->text() );
    project->setFileName( editProjectFile->text(), FALSE );
    project->setDescription( editProjectDescription->text() );
    project->setDatabaseDescription( editDatabaseFile->text() );
    project->setLanguage( comboLanguage->text( comboLanguage->currentItem() ) );
    project->save();
    accept();
}

void ProjectSettings::removeProject()
{
    QList<QListViewItem> lst;
    QListViewItemIterator it( listInterfaces );
    while ( it.current() ) {
	if ( it.current()->isSelected() )
	    lst.append( it.current() );
	++it;
    }
	
    for ( QListViewItem *i = lst.first(); i; i = lst.next() ) {
	QMap<QListViewItem*, FormWindow*>::Iterator fit = formMap.find( i );
	FormWindow *fw = 0;
	if ( fit != formMap.end() )
	    fw = *fit;
	project->removeUiFile( i->text( 1 ), fw );
	if ( fw ) {
	    fw->commandHistory()->setModified( FALSE );
	    fw->close();
	}
    }

    lst.setAutoDelete( TRUE );
}
