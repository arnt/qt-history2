/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qaction.h>
#include <qaxfactory.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qtextedit.h>

#include "menus.h"

#include "fileopen.xpm"
#include "filesave.xpm"

QMenus::QMenus( QWidget *parent, const char *name ) 
    : QMainWindow( parent, name, 0 ) // QMainWindow's default flag is WType_TopLevel
{
    QAction *action;

    QPopupMenu *file = new QPopupMenu( this );

    action = new QAction( "Open...", QPixmap( (const char**)fileopen ),
	"&Open", CTRL+Key_O, this );
    connect( action, SIGNAL(activated()), this, SLOT(fileOpen()) );
    action->addTo( file );

    action = new QAction( "Save", QPixmap( (const char**)filesave ),
	"&Save", CTRL+Key_S, this );
    connect( action, SIGNAL(activated()), this, SLOT(fileSave()) );
    action->addTo( file );


    QPopupMenu *edit = new QPopupMenu( this );

    action = new QAction( "Normal", "&Normal", CTRL+Key_N, this );
    connect( action, SIGNAL(activated()), this, SLOT(editNormal()) );
    action->addTo( edit );

    action = new QAction( "Bold", "&Bold", CTRL+Key_B, this );
    connect( action, SIGNAL(activated()), this, SLOT(editBold()) );
    action->addTo( edit );

    action = new QAction( "Underline", "&Underline", CTRL+Key_U, this );
    connect( action, SIGNAL(activated()), this, SLOT(editUnderline()) );
    action->addTo( edit );

    QPopupMenu *help = new QPopupMenu( this );

    action = new QAction( "About", "&About...", Key_F1, this );
    connect( action, SIGNAL(activated()), this, SLOT(helpAbout()) );
    action->addTo( help );

    action = new QAction( "About Qt", "&About Qt...", 0, this );
    connect( action, SIGNAL(activated()), this, SLOT(helpAboutQt()) );
    action->addTo( help );

    if ( !QAxFactory::isServer() )
	menuBar()->insertItem( "&File", file );
    menuBar()->insertItem( "&Edit", edit );
    menuBar()->insertItem( "&Help", help );

    editor = new QTextEdit( this, "editor" );
    setCentralWidget( editor );

    statusBar();
}

void QMenus::fileOpen()
{
    editor->append( "File Open selected." );
}

void QMenus::fileSave()
{
    editor->append( "File Save selected." );
}

void QMenus::editNormal()
{
    editor->append( "Edit Normal selected." );
}

void QMenus::editBold()
{
    editor->append( "Edit Bold selected." );
}

void QMenus::editUnderline()
{
    editor->append( "Edit Underline selected." );
}

void QMenus::helpAbout()
{
    QMessageBox::about( this, "About QMenus", "" );
}

void QMenus::helpAboutQt()
{
    QMessageBox::aboutQt( this );
}
