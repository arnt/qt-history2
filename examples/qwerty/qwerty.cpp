/****************************************************************************
** $Id: //depot/qt/main/examples/qwerty/qwerty.cpp#4 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "qwerty.h"
#include <qapplication.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qkeycode.h>
#include <qpopupmenu.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <qmessagebox.h>
#include <qpaintdevicemetrics.h>
#include <qlist.h>

#include <qtextcodec.h>


static QList<QTextCodec> *codecList = 0;

enum { Uni = 0, MBug = 1, Lat1 = 2, Codec = 5 };


Editor::Editor( QWidget * parent , const char * name )
    : QWidget( parent, name, WDestructiveClose )
{
    m = new QMenuBar( this, "menu" );
    QPopupMenu * file = new QPopupMenu();
    CHECK_PTR( file );
    m->insertItem( "&File", file );

    file->insertItem( "&New",   this, SLOT(newDoc()),   ALT+Key_N );
    file->insertItem( "&Open",  this, SLOT(load()),     ALT+Key_O );
    file->insertItem( "&Save",  this, SLOT(save()),     ALT+Key_S );

    save_as = new QPopupMenu();
    file->insertItem( "Save &as",  save_as );
    file->insertItem( "Add &encoding", this, SLOT(addEncoding()) );
    file->insertSeparator();
    file->insertItem( "&Print", this, SLOT(print()),    ALT+Key_P );
    file->insertSeparator();
    file->insertItem( "&Close", this, SLOT(close()),ALT+Key_W );
    file->insertItem( "&Quit",  qApp, SLOT(closeAllWindows()),     ALT+Key_Q );

    connect( save_as, SIGNAL(activated(int)), this, SLOT(saveAsEncoding(int)) );
    rebuildCodecList();

    changed = FALSE;
    e = new QMultiLineEdit( this, "editor" );
    connect( e, SIGNAL( textChanged() ), this, SLOT( textChanged() ) );
    e->setFont( QFont("Helvetica", 24) );

    e->setFocus();
}

Editor::~Editor()
{
}

void Editor::rebuildCodecList()
{
    delete codecList;
    codecList = new QList<QTextCodec>;
    QTextCodec *codec;
    int i;
    for (i = 0; (codec = QTextCodec::codecForIndex(i)); i++)
	codecList->append( codec );
    int n = codecList->count();
    save_as->clear();
    save_as->insertItem("Unicode", Uni );
    save_as->insertItem("Latin1", Lat1 );
    save_as->insertItem("Microsoft Unicode", MBug );
    for ( i = 0; i < n; i++ )
	save_as->insertItem( codecList->at(i)->name(), Codec + i );
}

void Editor::newDoc()
{
    Editor *ed = new Editor;
    ed->resize( 400, 400 );
    ed->show();
}


void Editor::load()
{
    QString fn = QFileDialog::getOpenFileName( QString::null, QString::null, this );
    if ( !fn.isEmpty() )
	load( fn );
}

void Editor::load( const QString& fileName )
{
    QFile f( fileName );
    if ( !f.open( IO_ReadOnly ) )
	return;

    e->setAutoUpdate( FALSE );

    QTextStream t(&f);
    e->setText( t.read() );
    f.close();

    e->setAutoUpdate( TRUE );
    e->repaint();
    setCaption( fileName );

    changed = FALSE;
}

bool Editor::save()
{
    //storing filename (proper save) is left as an exercise...
    QString fn = QFileDialog::getSaveFileName( QString::null, QString::null, this );
    if ( !fn.isEmpty() )
	return saveAs( fn );
    return FALSE;
}

void Editor::saveAsEncoding( int code )
{
    //storing filename (proper save) is left as an exercise...
    QString fn = QFileDialog::getSaveFileName( QString::null, QString::null, this );
    if ( !fn.isEmpty() )
	(void) saveAs( fn, code );
}

void Editor::addEncoding()
{
    QString fn = QFileDialog::getOpenFileName( QString::null, "*.map", this );
    if ( !fn.isEmpty() ) {
	QFile f(fn);
	if (f.open(IO_ReadOnly)) {
	    if (QTextCodec::loadCharmap(&f)) {
		rebuildCodecList();
	    } else {
		QMessageBox::warning(0,"Charmap error",
		    "The file did not contain a valid charmap.\n\n"
		    "A charmap file should look like this:\n"
		       "  <code_set_name> thename\n"
		       "  <escape_char> /\n"
		       "  % alias thealias\n"
		       "  CHARMAP\n"
		       "  <tokenname> /x12 <U3456>\n"
		       "  <tokenname> /xAB/x12 <U0023>\n"
		       "  ...\n"
		       "  END CHARMAP\n"
		);
	    }
	}
    }
}


bool Editor::saveAs( const QString& fileName, int code )
{
    QFile f( fileName );
    if ( !f.open( IO_WriteOnly ) ) {
	QMessageBox::warning(this,"I/O Error",
		    QString("The file could not be opened.\n\n")
			+fileName);
	return FALSE;
    }
    QTextStream t(&f);
    if ( code >= Codec )
	t.setCodec( codecList->at(code-Codec) );
    else if ( code == Uni )
	t.setEncoding( QTextStream::Unicode );
    else if ( code == MBug )
	t.setEncoding( QTextStream::UnicodeReverse );
    else if ( code == Lat1 )
	t.setEncoding( QTextStream::Latin1 );
    t << e->text();
    f.close();
    setCaption( fileName );
    changed = FALSE;
    return TRUE;
}


void Editor::print()
{
    const int MARGIN = 10;

    if ( printer.setup(this) ) {		// opens printer dialog
	QPainter p;
	p.begin( &printer );			// paint on printer
	p.setFont( e->font() );
	int yPos        = 0;			// y position for each line
	QFontMetrics fm = p.fontMetrics();
	QPaintDeviceMetrics metrics( &printer ); // need width/height
	                                         // of printer surface
	for( int i = 0 ; i < e->numLines() ; i++ ) {
	    if ( MARGIN + yPos > metrics.height() - MARGIN ) {
		printer.newPage();		// no more room on this page
		yPos = 0;			// back to top of page
	    }
	    p.drawText( MARGIN, MARGIN + yPos,
			metrics.width(), fm.lineSpacing(),
			ExpandTabs | DontClip,
			e->textLine( i ) );
	    yPos = yPos + fm.lineSpacing();
	}
	p.end();				// send job to printer
    }
}


void Editor::resizeEvent( QResizeEvent * )
{
    if ( e && m )
	e->setGeometry( 0, m->height(), width(), height() - m->height() );
}

void Editor::closeEvent( QCloseEvent *e )
{
    e->accept();

    if ( changed ) { // the text has been changed
	switch ( QMessageBox::warning( this, "Qwerty",
					"Save changes to Document?",
					tr("&Yes"),
					tr("&No"),
					tr("Cancel"),
					0, 2) ) {
	case 0: // yes
	    if ( save() )
		e->accept();
	    else
		e->ignore();
	    break;
	case 1: // no
	    e->accept();
	    break;
	default: // cancel
	    e->ignore();
	    break;
	}
    }
}

void Editor::textChanged()
{
    changed = TRUE;
}
