/*
  mainwindow.cpp
*/

#include <qapplication.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qstatusbar.h>
#include <qtextedit.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qworkspace.h>

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    QMainWindow w;
    w.setCaption( "QMainWindow" );

    QPopupMenu *file = new QPopupMenu;
    QPopupMenu *edit = new QPopupMenu;
    QPopupMenu *help = new QPopupMenu;

    QMenuBar *menuBar = w.menuBar();
    menuBar->insertItem( "&File", file );
    menuBar->insertItem( "&Edit", edit );
    menuBar->insertSeparator();
    menuBar->insertItem( "&Help", help );

    QToolBar *toolbar1 = new QToolBar( "Toolbar 1", &w );
    (void) new QToolButton( QPixmap("fileopen.png"), "Open", 0, 0, 0, toolbar1 );
    (void) new QToolButton( QPixmap("filesave.png"), "Save", 0, 0, 0, toolbar1 );

    QToolBar *toolbar2 = new QToolBar( "Toolbar 2", &w );
    (void) new QToolButton( QPixmap("undo.png"), "Undo", 0, 0, 0, toolbar2 );
    (void) new QToolButton( QPixmap("redo.png"), "Redo", 0, 0, 0, toolbar2 );
    (void) new QToolButton( QPixmap("editcut.png"), "Cut", 0, 0, 0, toolbar2 );
    (void) new QToolButton( QPixmap("editcopy.png"), "Copy", 0, 0, 0, toolbar2 );
    (void) new QToolButton( QPixmap("editpaste.png"), "Paste", 0, 0, 0, toolbar2 );

#if 0
    QToolBar *toolbar3 = new QToolBar( "Toolbar 3", &w );
    (void) new QToolButton( QPixmap("undo.png"), "Undo", 0, 0, 0, toolbar3 );
    (void) QLabel( "Font:", toolbar3 );
    (void) QComboBox( TRUE, toolbar3 );
    (void) new QToolButton( QPixmap("undo.png"), "Undo", 0, 0, 0, toolbar3 );
#endif

    QWorkspace *mdi = new QWorkspace( &w );
    w.setCentralWidget( mdi );

    QTextEdit *doc1 = new QTextEdit( mdi );
    doc1->setCaption( "MDI Child 1" );
    doc1->setText( "<p>,,Die Mathematiker sind eine Art Franzosen: redet man zu"
		   " ihnen, so übersetzen sie es in ihre Sprache, und dann ist"
		   " es alsobald ganz etwas Anders.``"
		   "<p align=\"right\"><i>Goethe</i>" );

    QTextEdit *doc2 = new QTextEdit( mdi );
    doc2->setCaption( "MDI Child 2" );
    doc2->setText( "<p>``Mathematicians are like Frenchmen: whenever you say"
		   " something to them, they translate it into their own"
		   " language, and at once it is something entirely"
		   " different.''"
		   "<p align=\"right\"><i>Goethe</i>" );

    w.statusBar()->message( "Status message" );
    w.statusBar()->addWidget( new QLabel(" Indicator 1", w.statusBar()), 0,
			      TRUE );
    w.statusBar()->addWidget( new QLabel(" Indicator 2", w.statusBar()), 0,
			      TRUE );

    w.show();
    app.setMainWidget( &w );
    return app.exec();
}
