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

#include <qiconview.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qregexp.h>
#include <qpushbutton.h>
#include <stdlib.h>
#include <qcombobox.h>


NewForm::NewForm( QWidget *parent, const QStringList& projects,
		  const QString& currentProject, const QString &templatePath )
    : NewFormBase( parent, 0, TRUE ), templPath( templatePath )
{
    connect( helpButton, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );

    projectCombo->insertStringList( projects );
    projectCombo->setCurrentText( currentProject );

    QIconViewItem *i;
    QStringList languages = MetaDataBase::languages();
    for ( QStringList::Iterator it = languages.begin(); it != languages.end(); ++it ) {
	i = new QIconViewItem( templateView );
	i->setText( (*it) + " " + tr( "Project" ) );
	i->setPixmap( PixmapChooser::loadPixmap( "project.xpm" ) );
	i->setDragEnabled( FALSE );
    }

    i = new QIconViewItem( templateView );
    i->setText( tr( "Sourcefile" ) );
    i->setPixmap( PixmapChooser::loadPixmap( "filenew.xpm" ) );
    i->setDragEnabled( FALSE );

    i = new QIconViewItem( templateView );
    i->setText( tr( "Dialog" ) );
    i->setPixmap( PixmapChooser::loadPixmap( "newform.xpm" ) );
    i->setDragEnabled( FALSE );
    i = new QIconViewItem( templateView );
    i->setText( tr( "Wizard" ) );
    i->setPixmap( PixmapChooser::loadPixmap( "newform.xpm" ) );
    i->setDragEnabled( FALSE );
    i = new QIconViewItem( templateView );
    i->setText( tr( "Widget" ) );
    i->setPixmap( PixmapChooser::loadPixmap( "newform.xpm" ) );
    i->setDragEnabled( FALSE );
    i = new QIconViewItem( templateView );
    i->setText( tr( "Mainwindow" ) );
    i->setPixmap( PixmapChooser::loadPixmap( "newform.xpm" ) );
    i->setDragEnabled( FALSE );


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
	    i = new QIconViewItem( templateView );
	    i->setDragEnabled( FALSE );
	    QString name = fi->baseName();
	    name = name.replace( QRegExp( "_" ), " " );
	    i->setText( name );
	    i->setPixmap( PixmapChooser::loadPixmap( "newform.xpm" ) );
	}
    }

    templateView->setCurrentItem( templateView->firstItem() );
    templateView->viewport()->setFocus();
}

QString NewForm::project() const
{
    return projectCombo->currentText();
}

void NewForm::accept()
{
    NewFormBase::accept();
}

void NewForm::currentChanged(QIconViewItem* )
{
}


NewForm::Form NewForm::formType() const
{
    if ( templateView->currentItem()->text().endsWith( tr( "Project" ) ) )
	 return Project; //UUUUUUUUUGGGGGGLLYYYYYYYYYY############ and only temporary. TODOMATTHIAS
    if ( templateView->currentItem()->text() == tr( "Dialog" ) )
	return Dialog;
    if ( templateView->currentItem()->text() == tr( "Wizard" ) )
	return Wizard;
    if ( templateView->currentItem()->text() == tr( "Widget" ) )
	return Widget;
    if ( templateView->currentItem()->text() == tr( "Mainwindow" ) )
	return Mainwindow;
    return Custom;
}

QString NewForm::templateFile() const
{
    QString fn = "/" + templateView->currentItem()->text();
    fn.prepend( templPath );
    fn.append( ".ui" );
    fn = fn.replace( QRegExp( " " ), "_" );
    return fn;
}
