/****************************************************************************
** $Id: //depot/qt/main/examples/notepad/notepad.cpp#4 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qinputdialog.h>
#include <qmultilineedit.h>
#include <qcombobox.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>

#include "../compact/keyboard.h"

#include "notepad.h"


Note::Note( const QString &theTitle, const QString &theNote )
    : mTitle( theTitle ), mNote( theNote )
{
}

Note::~Note()
{
    save();
}

bool Note::load()
{
    QFile file( mTitle + ".note" );
    if ( file.open( IO_ReadOnly ) ) {
	QTextStream ts( &file );
	mNote = ts.read();
	return TRUE;
    }

    return FALSE;
}

bool Note::save()
{
    QFile file( mTitle + ".note" );
    if ( file.open( IO_WriteOnly ) ) {
	QTextStream ts( &file );
	ts << mNote;
	return TRUE;
    }

    return FALSE;
}


NotePad::NotePad( QWidget *parent, const char *name )
    : QMainWindow( parent, name )
{
    QToolBar *bar = new QToolBar( this );
    addToolBar( bar );
    
    QToolButton *tb = new QToolButton( QPixmap(), "New", "New", this,
				       SLOT(newNote()), bar );
    tb->setAutoRaise( FALSE );
    tb->setText( "New" );
    
    tb = new QToolButton( QPixmap(), "Delete", "Delete", this,
			  SLOT(deleteNote()), bar );
    tb->setAutoRaise( FALSE );
    tb->setText( "Delete" );

    noteList = new QComboBox( bar );
    connect( noteList, SIGNAL(activated(int)), this, SLOT(loadNote(int)) );

    edit = new QMultiLineEdit( this );
    setCentralWidget( edit );
    QFont f( "helvetica", 12 );
    edit->setFont( f );

#ifdef _WS_QWS_	// Qt/Embedded specific
    Keyboard *kbd = new Keyboard( 0, 0, WStyle_Customize
				    | WStyle_Tool | WStyle_StaysOnTop );
    kbd->resize( qApp->desktop()->width(), kbd->sizeHint().height() + 1 );
    kbd->move( 0, qApp->desktop()->height() - kbd->height() );
    kbd->show();
    setGeometry( 0, 0, qApp->desktop()->width(),
		 qApp->desktop()->height() - kbd->height() );
#endif

    currentNote = 0;
    loadTitles();
}

NotePad::~NotePad()
{
    if ( currentNote && edit->edited() )
	currentNote->setNote( edit->text() );
    delete currentNote;
}

void NotePad::loadTitles()
{
    QDir dir( ".", "*.note" );
    notes = dir.entryList();

    // remove the .note extension
    for ( QStringList::Iterator it = notes.begin(); it != notes.end(); ++it ) {
	(*it).truncate( (*it).findRev( '.' ) );
    }

    noteList->clear();
    noteList->insertStringList( notes );

    if ( noteList->count() ) {
	noteList->setCurrentItem( 0 );
	loadNote( 0 );
    }
}

void NotePad::newNote()
{
    bool ok = FALSE;
    QString title = QInputDialog::getText( "New Note", "Note Title",
			QString::null, &ok, this );

    if ( ok && !title.isEmpty() ) {
	if ( currentNote ) {
	    if ( edit->edited() )
		currentNote->setNote( edit->text() );
	    delete currentNote;
	    edit->setText( "" );
	}
	currentNote = new Note( title, edit->text() );
	edit->setEdited( TRUE );
	notes.append( title );
	noteList->insertItem( title );
	noteList->setCurrentItem( noteList->count() - 1 );
    }
}

void NotePad::deleteNote()
{
    if ( currentNote ) {
	QString title = currentNote->title();
	delete currentNote;
	currentNote = 0;
	QFile file( title + ".note" );
	file.remove();
	loadTitles();
    }
}

void NotePad::loadNote( int idx )
{
    if ( currentNote && edit->edited() )
	currentNote->setNote( edit->text() );
    delete currentNote;
    currentNote = new Note( notes[idx] );
    if ( currentNote->load() ) {
	edit->setText( currentNote->note() );
    } else {
	edit->setText( "" );
    }
}

int main( int argc, char *argv[] )
{
#ifdef __MIPSEL__
    // MIPSEL-specific init - make sure /proc exists for shm
    if ( mount("none","/proc","proc",0,0) ) {
	perror("Mounting - /proc");
    }
    if ( mount("none","/mnt","shm",0,0) ) {
	perror("Mounting - shm");
    }
#endif

    QApplication app( argc, argv, QApplication::GuiServer );

    QFont f( "helvetica", 10 );
    app.setFont( f );

    NotePad *notePad = new NotePad;
    notePad->show();

    app.setMainWidget( notePad );

    return app.exec();
}


