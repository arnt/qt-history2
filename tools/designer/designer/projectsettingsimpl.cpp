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
#include "metadatabase.h"

#include <qlineedit.h>
#include <qtextedit.h>
#include <qlistbox.h>
#include <qfiledialog.h>
#include <qcombobox.h>

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

    listInterfaces->clear();
    listInterfaces->insertStringList( pro->uiFiles() );
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

    QStringList lst;
    for ( int i = 0; i < (int)listInterfaces->count(); ++i )
	lst << listInterfaces->text( i );
    project->setUiFiles( lst );
    project->setDatabaseDescription( editDatabaseFile->text() );
    project->setLanguage( comboLanguage->text( comboLanguage->currentItem() ) );
    project->save();
    accept();
}

void ProjectSettings::removeUiFile()
{
    QStringList lst;
    for ( int i = 0; i < (int)listInterfaces->count(); ++i ) {
	if ( listInterfaces->item( i )->selected() )
	    continue;
	lst << listInterfaces->text( i );
    }
    listInterfaces->clear();
    listInterfaces->insertStringList( lst );
}

void ProjectSettings::addUiFile()
{
    QStringList lst = QFileDialog::getOpenFileNames( tr( "Project Files (*.ui);;All Files (*)" ), QString::null, this );
    if ( lst.isEmpty() )
	return;
    listInterfaces->insertStringList( lst );
}
