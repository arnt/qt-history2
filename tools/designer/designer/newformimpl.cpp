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

#include "newformimpl.h"
#include "mainwindow.h"
#include "pixmapchooser.h"
#include "metadatabase.h"
#include "project.h"
#include "formwindow.h"
#include "widgetfactory.h"
#include "widgetdatabase.h"
#include "actioneditorimpl.h"
#include "hierarchyview.h"
#include "resource.h"
#include "projectsettingsimpl.h"
#include "sourcefile.h"

#include <qiconview.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qregexp.h>
#include <qpushbutton.h>
#include <stdlib.h>
#include <qcombobox.h>
#include <qworkspace.h>
#include <qmessagebox.h>

static int forms = 0;

ProjectItem::ProjectItem( QIconView *view, const QString &text )
    : NewItem( view, text )
{
}

void ProjectItem::insert( Project * )
{
    MainWindow::self->createNewProject( lang );
}



FormItem::FormItem( QIconView *view, const QString &text )
    : NewItem( view, text )
{
}

void FormItem::insert( Project *pro )
{
    QString n = "Form" + QString::number( ++forms );
    FormWindow *fw = 0;
    fw = new FormWindow( MainWindow::self, MainWindow::self->qWorkspace(), n );
    fw->setProject( pro );
    FormFile *ff = new FormFile( FormFile::createUnnamedFileName(), TRUE, pro );
    ff->setFormWindow( fw );
    MetaDataBase::addEntry( fw );
    if ( fType == Widget ) {
	QWidget *w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QWidget" ),
					    fw, n.latin1() );
	fw->setMainContainer( w );
    } else if ( fType == Dialog ) {
	QWidget *w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QDialog" ), fw, n.latin1() );
	fw->setMainContainer( w );
    } else if ( fType == Wizard ) {
	QWidget *w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QWizard" ),
					    fw, n.latin1() );
	fw->setMainContainer( w );
    } else if ( fType == MainWindow ) {
	QWidget *w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QMainWindow" ),
					    fw, n.latin1() );
	fw->setMainContainer( w );
    }

    fw->setCaption( n );
    fw->resize( 600, 480 );
    MainWindow::self->insertFormWindow( fw );

    TemplateWizardInterface *iface =
	MainWindow::self->templateWizardInterface( fw->mainContainer()->className() );
    if ( iface ) {
	iface->setup( fw->mainContainer()->className(), fw->mainContainer(),
		      fw->iFace(), MainWindow::self->designerInterface() );
	iface->release();
    }

    // the wizard might have changed a lot, lets update everything
    MainWindow::self->actioneditor()->setFormWindow( fw );
    MainWindow::self->objectHierarchy()->setFormWindow( fw, fw );
    MainWindow::self->objectHierarchy()->formDefinitionView()->refresh();
    fw->killAccels( fw );
    fw->project()->setModified( TRUE );
    fw->setFocus();
    if ( !pro->isDummy() ) {
	fw->setSavePixmapInProject( TRUE );
	fw->setSavePixmapInline( FALSE );
    }
}



CustomFormItem::CustomFormItem( QIconView *view, const QString &text )
    : NewItem( view, text )
{
}

static void unifyFormName( FormWindow *fw, QWorkspace *qworkspace )
{
    QStringList lst;
    QWidgetList windows = qworkspace->windowList();
    for ( QWidget *w =windows.first(); w; w = windows.next() ) {
	if ( w == fw )
	    continue;
	lst << w->name();
    }

    if ( lst.findIndex( fw->name() ) == -1 )
	return;
    QString origName = fw->name();
    QString n = origName;
    int i = 1;
    while ( lst.findIndex( n ) != -1 ) {
	n = origName + QString::number( i++ );
    }
    fw->setName( n );
    fw->setCaption( n );
}

void CustomFormItem::insert( Project *pro )
{
    QString filename = templateFileName();
    if ( !filename.isEmpty() && QFile::exists( filename ) ) {
	Resource resource( MainWindow::self );
	if ( !resource.load( filename, FALSE ) ) {
	    QMessageBox::information( MainWindow::self, MainWindow::tr("Load Template"),
				      MainWindow::tr("Couldn't load form description from template " +
						     filename ) );
	    return;
	}
	if ( MainWindow::self->formWindow() ) {
	    MainWindow::self->formWindow()->setFileName( QString::null );
	    unifyFormName( MainWindow::self->formWindow(), MainWindow::self->qWorkspace() );
	    if ( !pro->isDummy() ) {
		MainWindow::self->formWindow()->setSavePixmapInProject( TRUE );
		MainWindow::self->formWindow()->setSavePixmapInline( FALSE );
	    }
	}
    }
}



SourceFileItem::SourceFileItem( QIconView *view, const QString &text )
    : NewItem( view, text ), visible( TRUE )
{
}

void SourceFileItem::insert( Project *pro )
{
    SourceFile *f = new SourceFile( SourceFile::createUnnamedFileName( ext ), TRUE, pro );
    MainWindow::self->editSource( f );
}

void SourceFileItem::setProject( Project *pro )
{
    bool v = lang == pro->language();
    if ( v == visible )
	return;
    visible = v;
    if ( !visible )
	iconView()->takeItem( this );
    else
	iconView()->insertItem( this );
}



NewForm::NewForm( QWidget *parent, const QStringList& projects,
		  const QString& currentProject, const QString &templatePath )
    : NewFormBase( parent, 0, TRUE )
{
    connect( helpButton, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );

    projectCombo->insertStringList( projects );
    projectCombo->setCurrentText( currentProject );

    QStringList languages = MetaDataBase::languages();
    QStringList::Iterator it;
    for ( it = languages.begin(); it != languages.end(); ++it ) {
	ProjectItem *pi = new ProjectItem( templateView, *it + " " + tr( "Project" ) );
	allItems.append( pi );
	pi->setLanguage( *it );
	pi->setPixmap( PixmapChooser::loadPixmap( "project.xpm" ) );
	pi->setDragEnabled( FALSE );
    }

    FormItem *fi = new FormItem( templateView,tr( "Dialog" ) );
    allItems.append( fi );
    fi->setFormType( FormItem::Dialog );
    fi->setPixmap( PixmapChooser::loadPixmap( "newform.xpm" ) );
    fi->setDragEnabled( FALSE );
    fi = new FormItem( templateView,tr( "Wizard" ) );
    allItems.append( fi );
    fi->setFormType( FormItem::Wizard );
    fi->setPixmap( PixmapChooser::loadPixmap( "newform.xpm" ) );
    fi->setDragEnabled( FALSE );
    fi = new FormItem( templateView, tr( "Widget" ) );
    allItems.append( fi );
    fi->setFormType( FormItem::Widget );
    fi->setPixmap( PixmapChooser::loadPixmap( "newform.xpm" ) );
    fi->setDragEnabled( FALSE );
    fi = new FormItem( templateView, tr( "Mainwindow" ) );
    allItems.append( fi );
    fi->setFormType( FormItem::MainWindow );
    fi->setPixmap( PixmapChooser::loadPixmap( "newform.xpm" ) );
    fi->setDragEnabled( FALSE );

    QString templPath = templatePath;
    if ( templPath.isEmpty() || !QFileInfo( templPath ).exists() ) {
	if ( QFileInfo( "../templates" ).exists() ) {
	    templPath = "../templates";
	} else {
	    QString qtdir = getenv( "QTDIR" );
	    if ( QFileInfo( qtdir + "/tools/designer/templates" ).exists() )
		templPath = qtdir + "/tools/designer/templates";
	}
    }

    QDir dir( templPath  );
    const QFileInfoList *filist = dir.entryInfoList( QDir::DefaultFilter, QDir::DirsFirst | QDir::Name );
    if ( filist ) {
	QFileInfoListIterator it( *filist );
	QFileInfo *fi;
	while ( ( fi = it.current() ) != 0 ) {
	    ++it;
	    if ( !fi->isFile() )
		continue;
	    QString name = fi->baseName();
	    name = name.replace( QRegExp( "_" ), " " );
	    CustomFormItem *ci = new CustomFormItem( templateView, name );
	    allItems.append( ci );
	    ci->setDragEnabled( FALSE );
	    ci->setPixmap( PixmapChooser::loadPixmap( "newform.xpm" ) );
	    ci->setTemplateFile( fi->absFilePath() );
	}
    }

    for ( it = languages.begin(); it != languages.end(); ++it ) {
	LanguageInterface *iface = MetaDataBase::languageInterface( *it );
	if ( iface ) {
	    QMap<QString, QString> extensionMap;
	    iface->preferedExtensions( extensionMap );
	    for ( QMap<QString, QString>::Iterator eit = extensionMap.begin();
		  eit != extensionMap.end(); ++eit ) {
		SourceFileItem * si = new SourceFileItem( templateView, *eit );
		allItems.append( si );
		si->setExtension( eit.key() );
		si->setLanguage( *it );
		si->setPixmap( PixmapChooser::loadPixmap( "filenew.xpm" ) );
		si->setDragEnabled( FALSE );
	    }
	}
    }

    templateView->setCurrentItem( templateView->firstItem() );
    templateView->viewport()->setFocus();

    projectChanged( projectCombo->currentText() );
}

void NewForm::accept()
{
    if ( !templateView->currentItem() )
	return;
    Project *pro = MainWindow::self->findProject( projectCombo->currentText() );
    if ( !pro )
	return;
    MainWindow::self->setCurrentProject( pro );
    NewFormBase::accept();
    ( (NewItem*)templateView->currentItem() )->insert( pro );
}

void NewForm::projectChanged( const QString &project )
{
    Project *pro = MainWindow::self->findProject( project );
    if ( !pro )
	return;
    QIconViewItem *i;
    for ( i = allItems.first(); i; i = allItems.next() )
	( (NewItem*)i )->setProject( pro );
    templateView->setCurrentItem( templateView->firstItem() );
    templateView->arrangeItemsInGrid( TRUE );
}
