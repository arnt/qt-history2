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

#include <qiconview.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qregexp.h>
#include <qpushbutton.h>
#include <stdlib.h>

static QString templatePath( const QString &t )
{
    if ( QFileInfo( "../templates" ).exists() )
	return "../templates";
    QString qtdir = getenv( "QTDIR" );
    if ( QFileInfo( qtdir + "/tools/designer/templates" ).exists() )
	return qtdir + "/tools/designer/templates";
    return t;
}

NewForm::NewForm( QWidget *parent, const QString &tPath )
    : NewFormBase( parent, 0, TRUE ), templPath( tPath )
{
    connect( helpButton, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );
    QIconViewItem *i = new QIconViewItem( templateView );
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
#ifndef QT_NO_SQL
    i = new QIconViewItem( templateView );
    i->setText( tr( "SQL Widget" ) );
    i->setPixmap( PixmapChooser::loadPixmap( "newform.xpm" ) );
    i->setDragEnabled( FALSE );
    i = new QIconViewItem( templateView );
    i->setText( tr( "SQL Dialog" ) );
    i->setPixmap( PixmapChooser::loadPixmap( "newform.xpm" ) );
    i->setDragEnabled( FALSE );
#endif

    QString templateDir = templatePath( templPath );
    QDir dir( templateDir );
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

NewForm::Form NewForm::formType() const
{
    if ( templateView->currentItem()->text() == tr( "Dialog" ) )
	return Dialog;
    if ( templateView->currentItem()->text() == tr( "Wizard" ) )
	return Wizard;
    if ( templateView->currentItem()->text() == tr( "Widget" ) )
	return Widget;
    if ( templateView->currentItem()->text() == tr( "Mainwindow" ) )
	return Mainwindow;
    if ( templateView->currentItem()->text() == tr( "SQL Widget" ) )
	return SqlWidget;
    if ( templateView->currentItem()->text() == tr( "SQL Dialog" ) )
	return SqlDialog;
    return Custom;
}

QString NewForm::templateFile() const
{
    QString fn = "/" + templateView->currentItem()->text();
    fn.prepend( templatePath( templPath ) );
    fn.append( ".ui" );
    fn = fn.replace( QRegExp( " " ), "_" );
    return fn;
}
