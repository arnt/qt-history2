/****************************************************************************
** $Id: //depot/qt/main/examples/application/application.cpp#3 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "application.h"

#include <qimage.h>
#include <qpixmap.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qkeycode.h>
#include <qmultilineedit.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <qapplication.h>
#include <qaccel.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qwhatsthis.h>
#include <qmime.h>

#include "filesave.xpm"
#include "fileopen.xpm"
#include "fileprint.xpm"

const char * fileOpenText = "<img source=\"fileopen\"> "
"Click this button to open a <em>new file</em>. <br><br>"
"You can also select the <b>Open command</b> from the File menu.";
const char * fileSaveText = "Click this button to save the file you are "
"editing.  You will be prompted for a file name.\n\n"
"You can also select the Save command from the File menu.\n\n"
"Note that implementing this function is left as an exercise for the reader.";
const char * filePrintText = "Click this button to print the file you "
"are editing.\n\n"
"You can also select the Print command from the File menu.";

ApplicationWindow::ApplicationWindow()
    : QMainWindow( 0, "example application main window" ), filename( QString::null )
{
//     QFont fnt("Courier");
//     setFont(fnt);
    int id;

    printer = new QPrinter;
    printer->setMinMax( 1, 10 );
    QPixmap openIcon, saveIcon, printIcon;

    fileTools = new QToolBar( this, "file operations" );

    openIcon = QPixmap( fileopen );
    QToolButton * fileOpen = new QToolButton( openIcon, "Open File",
                                              QString::null, this, SLOT(load()),
                                              fileTools, "open file" );

    saveIcon = QPixmap( filesave );
    QToolButton * fileSave = new QToolButton( saveIcon, "Save File",
                                              QString::null,
                                              this, SLOT(save()),
                                              fileTools, "save file" );

    printIcon = QPixmap( fileprint );
    QToolButton * filePrint = new QToolButton( printIcon, "Print File",
                                               QString::null,
                                               this, SLOT(print()),
                                               fileTools, "print file" );

    (void)QWhatsThis::whatsThisButton( fileTools );

    QWhatsThis::add( fileOpen, fileOpenText );
    QMimeSourceFactory::defaultFactory()->setPixmap( "fileopen", openIcon ); // used by fileOpenText, see above
    QWhatsThis::add( fileSave, fileSaveText );
    QWhatsThis::add( filePrint, filePrintText );

    e = new QMultiLineEdit( this, "editor" );

    QPopupMenu * file = new QPopupMenu( this );
    menuBar()->insertItem( "&File", file );

    file->insertItem( "New", this, SLOT(newDoc()), CTRL+Key_N );

    id = file->insertItem( openIcon, "Open...", this, SLOT(load()), CTRL+Key_O );
    file->setWhatsThis( id, fileOpenText );

    id = file->insertItem( saveIcon, "Save", this, SLOT(save()), CTRL+Key_S );
    file->setWhatsThis( id, fileSaveText );
    id = file->insertItem( "Save as...", this, SLOT(saveAs()) );
    file->setWhatsThis( id, fileSaveText );
    file->insertSeparator();
    id = file->insertItem( printIcon, "Print...", this, SLOT(print()), CTRL+Key_P );
    file->setWhatsThis( id, filePrintText );
    file->insertSeparator();
    file->insertItem( "Close", this, SLOT(closeDoc()), CTRL+Key_W );
    file->insertItem( "Quit", qApp, SLOT(quit()), CTRL+Key_Q );

    QPopupMenu * edit = new QPopupMenu( this );
    menuBar()->insertItem( "&Edit", edit );

    edit->insertItem( "&Cut", e, SLOT(cut()), CTRL + Key_X );
    edit->insertItem( "C&opy", e, SLOT(copy()), CTRL + Key_C );
    edit->insertItem( "&Paste", e, SLOT(paste()), CTRL + Key_V );

    controls = new QPopupMenu( this );
    menuBar()->insertItem( "&Controls", controls );

    QPopupMenu * help = new QPopupMenu( this );
    menuBar()->insertSeparator();
    menuBar()->insertItem( "&Help", help );

    help->insertItem( "&About", this, SLOT(about()), Key_F1);
    help->insertItem( "About&Qt", this, SLOT(aboutQt()));
    help->insertSeparator();
    help->insertItem( "What's &This", this, SLOT(whatsThis()), SHIFT+Key_F1);

    mb = controls->insertItem( "Menu bar", this, SLOT(toggleMenuBar()), CTRL+Key_M);
    // Now an accelerator for when the menubar is invisible!
    QAccel* a = new QAccel(this);
    id = a->insertItem( CTRL+Key_M );
    a->setWhatsThis( id, "Toggle Menubar");
    a->connectItem( id , this, SLOT(toggleMenuBar()) );

    tb = controls->insertItem( "Tool bar", this, SLOT(toggleToolBar()), CTRL+Key_T);
    sb = controls->insertItem( "Status bar", this, SLOT(toggleStatusBar()), CTRL+Key_B);
    controls->setCheckable( TRUE );
    controls->setItemChecked( mb, TRUE );
    controls->setItemChecked( tb, TRUE );
    controls->setItemChecked( sb, TRUE );

    e->installEventFilter( this );
    e->setFocus();
    setCentralWidget( e );
    statusBar()->message( "Ready", 2000 );
}


ApplicationWindow::~ApplicationWindow()
{
    delete printer;
}



void ApplicationWindow::newDoc()
{
    ApplicationWindow *ed = new ApplicationWindow;
    ed->resize( 400, 400 );
    ed->show();
}

void ApplicationWindow::load()
{
    QString fn = QFileDialog::getOpenFileName( QString::null, QString::null, this );
    if ( !fn.isEmpty() )
        load( fn );
    else
        statusBar()->message( "Loading aborted", 2000 );
}


void ApplicationWindow::load( const QString &fileName )
{
    QFile f( fileName );
    if ( !f.open( IO_ReadOnly ) )
        return;

    e->setAutoUpdate( FALSE );
    e->clear();

    QTextStream t(&f);
    while ( !t.eof() ) {
        QString s = t.readLine();
        e->append( s );
    }
    f.close();

    e->setAutoUpdate( TRUE );
    e->repaint();
    setCaption( fileName );
    statusBar()->message( QString( "Loaded document %1" ).arg( fileName ), 2000 );

    filename = fileName;
}


void ApplicationWindow::save()
{
    if ( filename.isEmpty() ) {
        saveAs();
        return;
    }

    QString text = e->text();
    QFile f( filename );
    if ( !f.open( IO_WriteOnly ) )
        return;

    QTextStream t( &f );
    t << text;
    f.close();

    setCaption( filename );

    statusBar()->message( QString( "File %1 saved", 2000 ).arg( filename ) );
}

void ApplicationWindow::saveAs()
{
    QString fn = QFileDialog::getSaveFileName(QString::null,QString::null,this);
    if ( !fn.isEmpty() ) {
        filename = fn;
        save();
    } else {
        statusBar()->message( "Saving aborted", 2000 );
    }
}

void ApplicationWindow::print()
{
    const int MARGIN = 10;
    int pageNo = 1;

    if ( printer->setup( this ) ) {		// printer dialog
        statusBar()->message( "Printing..." );
        QPainter p;
        p.begin( printer );			// paint on printer
        p.setFont( e->font() );
        int yPos        = 0;			// y position for each line
        QFontMetrics fm = p.fontMetrics();
        QPaintDeviceMetrics metrics( printer ); // need width/height
        // of printer surface
        for( int i = 0 ; i < e->numLines() ; i++ ) {
            if ( MARGIN + yPos > metrics.height() - MARGIN ) {
                QString msg;
                msg.sprintf( "Printing (page %d)...", ++pageNo );
                statusBar()->message( msg );
                printer->newPage();		// no more room on this page
                yPos = 0;			// back to top of page
            }
            p.drawText( MARGIN, MARGIN + yPos,
                        metrics.width(), fm.lineSpacing(),
                        ExpandTabs | DontClip,
                        e->textLine( i ) );
            yPos = yPos + fm.lineSpacing();
        }
        p.end();				// send job to printer
        statusBar()->message( "Printing completed", 2000 );
    } else {
        statusBar()->message( "Printing aborted", 2000 );
    }

}

void ApplicationWindow::closeDoc()
{
    close( TRUE ); // close AND DELETE!
}


void ApplicationWindow::about()
{
    QMessageBox::about( this, "Qt Application Example",
                        "This example demonstrates simple use of QMainWindow,\n"
                        "QMenuBar and QToolBar.");
}


void ApplicationWindow::aboutQt()
{
    QMessageBox::aboutQt( this, "Qt Application Example" );
}


void ApplicationWindow::toggleMenuBar()
{
    if ( menuBar()->isVisible() ) {
        menuBar()->hide();
        controls->setItemChecked( mb, FALSE );
    } else {
        menuBar()->show();
        controls->setItemChecked( mb, TRUE );
    }
}

void ApplicationWindow::toggleToolBar()
{
    if ( fileTools->isVisible() ) {
        fileTools->hide();
        controls->setItemChecked( tb, FALSE );
    } else {
        fileTools->show();
        controls->setItemChecked( tb, TRUE );
    }
}

void ApplicationWindow::toggleStatusBar()
{
    if ( statusBar()->isVisible() ) {
        statusBar()->hide();
        controls->setItemChecked( sb, FALSE );
    } else {
        statusBar()->show();
        controls->setItemChecked( sb, TRUE );
    }
}

bool ApplicationWindow::eventFilter( QObject *o, QEvent * ev)
{
    // context menu for the multiline edit e

    // we could also have done that by subclassing QMultiLineEdit and
    // reimplementing the mousePressEvent(QMouseEvent*) function.

    if ( o == e && ev->type() == QEvent::MouseButtonPress) {
        QMouseEvent* me = (QMouseEvent*)ev;
        if ( me->button() == RightButton ) {
            controls->popup(me->globalPos() );
            return TRUE;
        }
    }
    return QMainWindow::eventFilter(o, ev);
}
