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
#include "asciivalidator.h"
#include "mainwindow.h"
#include "sourcefile.h"
#include "pixmapchooser.h"
#include "workspace.h"

#include <qlineedit.h>
#include <qtextedit.h>
#include <qlistbox.h>
#include <qfiledialog.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qobjectlist.h>
#include <qheader.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlabel.h>

static const char* file_xpm[]={
    "16 16 5 1",
    ". c #7f7f7f",
    "# c None",
    "c c #000000",
    "b c #bfbfbf",
    "a c #ffffff",
    "################",
    "..........######",
    ".aaaaaaaab.#####",
    ".aaaaaaaaba.####",
    ".aaaaaaaacccc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".bbbbbbbbbbbc###",
    "ccccccccccccc###"};

static QPixmap *filePixmap = 0;

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
    
    PushButton5->hide();
    PushButton4->hide();
    listInterfaces->hide();
    TextLabel3->hide();
    
    if ( !filePixmap )
	filePixmap = new QPixmap( file_xpm );
    editProjectFile->setFocus();

    if ( project->isDummy() ) {
	editProjectFile->setEnabled( FALSE );
	editProjectFile->setText( project->projectName() );
    } else {
	if ( pro->fileName().isEmpty() || pro->fileName() == ".pro" ) {
	    editProjectFile->setText( tr( "unnamed.pro" ) );
	    editProjectFile->selectAll();
	} else {
	    editProjectFile->setText( pro->fileName() );
	}
    }

    editProjectDescription->setText( pro->description() );

    fillFilesList();

    listInterfaces->header()->setStretchEnabled( TRUE );

    editDatabaseFile->setText( pro->databaseDescription() );

    comboLanguage->insertStringList( MetaDataBase::languages() );
    for ( int j = 0; j < (int)comboLanguage->count(); ++j ) {
	if ( project->language() == comboLanguage->text( j ) ) {
	    comboLanguage->setCurrentItem( j );
	    break;
	}
    }

    checkCreateSource->setChecked( pro->customSetting( "CPP_ALWAYS_CREATE_SOURCE" ) == "TRUE" );
    checkCreateSource->setEnabled( comboLanguage->currentText() == "C++" );
}

/*
 *  Destroys the object and frees any allocated resources
 */
ProjectSettings::~ProjectSettings()
{
}

void ProjectSettings::chooseDatabaseFile()
{
    QString fn = QFileDialog::getSaveFileName( QString::null, tr( "Database Files (*.db);;All Files (*)" ), this );
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
    project->setFileName( editProjectFile->text(), FALSE );
    project->setDescription( editProjectDescription->text() );
    project->setDatabaseDescription( editDatabaseFile->text() );
    project->setLanguage( comboLanguage->text( comboLanguage->currentItem() ) );
    QString flag = "FALSE";
    if ( checkCreateSource->isChecked() )
	flag = "TRUE";
    project->setCustomSetting( "CPP_ALWAYS_CREATE_SOURCE", flag );
    project->setModified( TRUE );
    accept();
}

void ProjectSettings::removeProject()
{
    /*
    QListViewItemIterator it( listInterfaces );
    while ( it.current() ) {
	QListViewItem *i = it.current();
	if ( !i->isSelected() ) {
	    ++it;
	    continue;
	}
	QMap<QListViewItem*, FormWindow*>::Iterator fit = formMap.find( i );
	if ( fit != formMap.end() ) {
	    MainWindow::self->workspace()->removeFormFromProject( i->text( 0 ) );
	} else {
	    QMap<QListViewItem*, SourceFile*>::Iterator sit = sourceMap.find( i );
	    if ( sit != sourceMap.end() ) {
		MainWindow::self->workspace()->removeSourceFromProject( i->text( 0 ) );
	    }
	}
	++it;
    }

    fillFilesList();
    */
}
void ProjectSettings::languageChanged( const QString &lang )
{
    checkCreateSource->setEnabled( lang == "C++" );
}

void ProjectSettings::addProject()
{
    /*
    QString filter = "Qt User-Interface Files (*.ui)";
    QString extensions = ";ui";
    LanguageInterface *iface = MetaDataBase::languageInterface( project->language() );
    if ( iface ) {
	QMap<QString, QString> extensionFilterMap;
	iface->fileFilters( extensionFilterMap );
	for ( QMap<QString,QString>::Iterator it = extensionFilterMap.begin();
	      it != extensionFilterMap.end(); ++it ) {
	    filter += ";;" + *it;
	    extensions += ";" + it.key();
	}
    }
    MainWindow::self->fileOpen( filter, extensions );
    fillFilesList();
    */
}

void ProjectSettings::fillFilesList()
{
    /*
    listInterfaces->clear();
    formMap.clear();
    sourceMap.clear();

    QListViewItem *sources = new QListViewItem( listInterfaces, tr( "Source Files" ) );
    QListViewItem *forms = new QListViewItem( listInterfaces, tr( "Forms" ) );
    forms->setOpen( TRUE );
    forms->setSelectable( FALSE );
    sources->setOpen( TRUE );
    sources->setSelectable( FALSE );

    QStringList lst = project->uiFiles();
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
	QListViewItem *item = new QListViewItem( forms, *it, tr( "<unknown>" ), 0 );
	item->setPixmap( 0, PixmapChooser::loadPixmap( "form.xpm", PixmapChooser::Mini ) );
	QString className = project->formName( item->text( 0 ) );
	FormWindow *fw = project->formWindow( item->text( 0 ) );
	formMap.insert( item, fw );
	if ( !className.isEmpty() )
	    item->setText( 1, className );
    }

    QPtrList<SourceFile> sourceList = project->sourceFiles();
    for ( SourceFile *f = sourceList.first(); f; f = sourceList.next() ) {
	QListViewItem *i = new QListViewItem( sources, project->makeRelative( f->fileName() ) );
	i->setPixmap( 0, *filePixmap );
	sourceMap.insert( i, f );
    }
    */
}
