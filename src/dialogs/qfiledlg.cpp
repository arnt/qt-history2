/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qfiledlg.cpp#2 $
**
** Implementation of QFileDialog class
**
** Author  : Eirik Eng
** Created : 950429
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qfiledlg.h"
#include "qlistbox.h"
#include "qlined.h"
#include "qcombo.h"
#include "qlabel.h"
#include "qpushbt.h"
#include "qmsgbox.h"
#include "qapp.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/dialogs/qfiledlg.cpp#2 $";
#endif


/*----------------------------------------------------------------------------
  \class QFileDialog qfiledlg.h
  \brief The QFileDialog provides a dialog widget for inputting file names.

  Example:
  \code
    QString fileName = QFileDialog::getOpenFileName();
    if ( !fileName.isNull() ) {			// got a file name
	...
    }
  \endcode
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Constructs a file dialog with a \e parent, \e name and \e modal flag.

  The dialog becomes modal if \e modal is TRUE, otherwise modeless.
 ----------------------------------------------------------------------------*/

QFileDialog::QFileDialog( QWidget *parent, const char *name, bool modal )
    : QDialog( parent, name, modal )
{
    init();
    filterEdit->setText( "*" );
    d.convertToAbsolute();
    rereadDir();
    resize( 300, 300 );
}

/*----------------------------------------------------------------------------
  Constructs a file dialog with a \e parent, \e name and \e modal flag.

  The dialog becomes modal if \e modal is TRUE, otherwise modeless.
 ----------------------------------------------------------------------------*/

QFileDialog::QFileDialog( const char *dirName, const char *filter,
			  QWidget *parent, const char *name, bool modal )
    : QDialog( parent, name, modal )
{
    init();
    if ( filter )
	d.setNameFilter( filter );
    filterEdit->setText( d.nameFilter() );
    if ( dirName )
	d.setPath( dirName );
    d.convertToAbsolute();
    rereadDir();
    resize( 300, 300 );
}

/*----------------------------------------------------------------------------
  \internal
  Initializes the file dialog.
 ----------------------------------------------------------------------------*/

void QFileDialog::init()
{
    files      = new QListBox(		     this, "fileList"	  );
    dirs       = new QListBox(		     this, "dirList"	  );
    filterEdit = new QLineEdit(		     this, "filterEdit"	  );
    nameEdit   = new QLineEdit(		     this, "nameEdit"	);
    pathBox    = new QComboBox(		     this, "pathBox"	  );
    filterL    = new QLabel( "Filter:"	   , this, "filterLabel"  );
    nameL      = new QLabel( "Name:"	   , this, "filterLabel"  );
    dirL       = new QLabel( "Directories:", this, "dirLabel"	  );
    fileL      = new QLabel( "Files:"	   , this, "fileLabel"	  );
    okB	       = new QPushButton( "OK"	   , this, "okButton"	  );
    filterB    = new QPushButton( "Filter" , this, "filterButton" );
    cancelB    = new QPushButton( "Cancel" , this, "cancelButton" );

    pathBox->setAutoResize( TRUE );
    filterL->setAutoResize( TRUE );
    nameL  ->setAutoResize( TRUE );
    dirL   ->setAutoResize( TRUE );
    fileL  ->setAutoResize( TRUE );

    dirs   ->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    dirs   ->setLineWidth( 1 );
    files  ->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    files  ->setLineWidth( 1 );

    okB	   ->resize( 50, 25 );
    filterB->resize( 50, 25 );
    cancelB->resize( 50, 25 );

    connect( files, 	SIGNAL(selected(int)), 	 SLOT(fileSelected(int)) );
    connect( files, 	SIGNAL(highlighted(int)),SLOT(fileHighlighted(int)) );
    connect( dirs, 	SIGNAL(selected(int)), 	 SLOT(dirSelected(int)) );
    connect( pathBox,	SIGNAL(activated(int)),  SLOT(pathSelected(int)) );
    connect( okB, 	SIGNAL(clicked()), 	 SLOT(okClicked()) );
    connect( nameEdit, 	SIGNAL(returnPressed()), SLOT(okClicked()) );
    connect( filterEdit,SIGNAL(returnPressed()), SLOT(filterClicked()) );
    connect( filterB, 	SIGNAL(clicked()), 	 SLOT(filterClicked()) );
    connect( cancelB, 	SIGNAL(clicked()), 	 SLOT(cancelClicked()) );
    d.setMatchAllDirs( TRUE );
    d.setSorting( d.sorting() | QDir::DirsFirst );
}

/*----------------------------------------------------------------------------
  Destroys the file dialog.
 ----------------------------------------------------------------------------*/

QFileDialog::~QFileDialog()
{
    delete files;
    delete dirs;
    delete filterEdit;
    delete nameEdit;
    delete pathBox;
    delete filterL;
    delete nameL;
    delete dirL;
    delete fileL;
    delete okB;
    delete filterB;
    delete cancelB;
}


/*----------------------------------------------------------------------------
  Returns the selected file name.

  If a file name was selected, the returned string will contain the full
  path name.
  The returned string will be a null string if no file name was selected.

  \sa QString::isNull()
 ----------------------------------------------------------------------------*/

QString QFileDialog::selectedFile() const
{
    QString tmp;
    if ( nameEdit->text() && strcmp( nameEdit->text(), "" ) != 0 )
	tmp = d.fullPathName( nameEdit->text() );
    return tmp;
}


/*----------------------------------------------------------------------------
  Returns the active directory path string in the file dialog.
  \sa setDir()
 ----------------------------------------------------------------------------*/

const char *QFileDialog::dirPath() const
{
    return d.path();
}

/*----------------------------------------------------------------------------
  Sets a directory path string for the file dialog.
  \sa dir()
 ----------------------------------------------------------------------------*/

void QFileDialog::setDir( const char *pathstr )
{
    if ( strcmp(d.path(),pathstr) == 0 )
	return;
    d.setPath( pathstr );
    d.convertToAbsolute();
    rereadDir();
}

/*----------------------------------------------------------------------------
  Returns the active directory in the file dialog.
  \sa setDir()
 ----------------------------------------------------------------------------*/

const QDir *QFileDialog::dir() const
{
    return &d;
}

/*----------------------------------------------------------------------------
  Sets a directory path for the file dialog.
  \sa dir()
 ----------------------------------------------------------------------------*/

void QFileDialog::setDir( const QDir &dir )
{
    d = dir;
    d.convertToAbsolute();
    d.setMatchAllDirs( TRUE );
    d.setSorting( d.sorting() | QDir::DirsFirst );
    rereadDir();
}


/*----------------------------------------------------------------------------
  Re-reads the active directory in the file dialog.

  It is normally not required to call this function, unless you suspect
  that the directory contents has changed and want to refresh the
  directory list box.
 ----------------------------------------------------------------------------*/

void QFileDialog::rereadDir()
{
    qApp ->setCursor( waitCursor );
    dirs ->setAutoUpdate( FALSE );
    files->setAutoUpdate( FALSE );
    dirs ->clear();
    files->clear();

    const QFileInfoList *filist = d.entryInfos();
    QFileInfoIterator 	 it( *filist );
    QFileInfo		*fi = it.current();
    while ( fi && fi->isDir() ) {
	dirs->insertItem( fi->fileName().data() );
	fi = ++it;
    }    
    while ( fi ) {
	files->insertItem( fi->fileName().data() );
	fi = ++it;
    }

    dirs ->setAutoUpdate( TRUE );
    files->setAutoUpdate( TRUE );
    dirs ->repaint();
    files->repaint();
    updatePathBox( d.path() );
    qApp->restoreCursor();
}


/*----------------------------------------------------------------------------
  Opens a modal file dialog and returns the name of the file to be opened.
  Returns a \link QString::isNull() null string\endlink if the user cancelled
  the dialog.
 ----------------------------------------------------------------------------*/

QString QFileDialog::getOpenFileName( const char *dirName, const char *filter,
				      QWidget *parent, const char *name )
{
    QFileDialog *dlg = new QFileDialog( dirName, filter, parent, name, TRUE );
    CHECK_PTR( dlg );
    dlg->setCaption( "Open" );
    QString result;
    if ( dlg->exec() == QDialog::Accepted )
	result = dlg->selectedFile();
    delete dlg;
    return result;
}

/*----------------------------------------------------------------------------
  Opens a modal file dialog and returns the name of the file to be saved.
  Returns a \link QString::isNull() null string\endlink if the user cancelled
  the dialog.
 ----------------------------------------------------------------------------*/

QString QFileDialog::getSaveFileName( const char *dirName, const char *filter,
				      QWidget *parent, const char *name )
{
    QFileDialog *dlg = new QFileDialog( dirName, filter, parent, name, TRUE );
    CHECK_PTR( dlg );
    dlg->setCaption( "Save As" );
    QString result;
    if ( dlg->exec() == QDialog::Accepted )
	result = dlg->selectedFile();
    delete dlg;
    return result;
}


/*----------------------------------------------------------------------------
  \internal
  Activated when a file name in the file list has been selected.
 ----------------------------------------------------------------------------*/

void QFileDialog::fileSelected( int index )
{
    nameEdit->setText( files->string( index ) );
    emit fileSelected( d.pathName( nameEdit->text() ) );
    accept();
}

/*----------------------------------------------------------------------------
  \internal
  Activated when a file name in the file list has been highlighted.
 ----------------------------------------------------------------------------*/

void QFileDialog::fileHighlighted( int index )
{
    nameEdit->setText( files->string( index ) );
    emit fileHighlighted( d.pathName( files->string( index ) ) );
}

/*----------------------------------------------------------------------------
  \internal
  Activated when a directory name in the directory list has been selected.
 ----------------------------------------------------------------------------*/

void QFileDialog::dirSelected( int index )
{
    QDir tmp = d;
    if ( d.cd( dirs->string(index) ) && d.isReadable() ) {
	nameEdit->setText( "" );
	emit dirEntered( d.path() );
	rereadDir();
    } else {
	QMessageBox::message( "Sorry", "Cannot open or read directory." );
	d = tmp;
    }	 
}

void QFileDialog::pathSelected( int index )
{
    if ( index == 0 )				// current directory shown
	return;
    QString newPath;
    QDir tmp = d;
    for( int i = pathBox->count() - 1 ; i >= index ; i-- )
	newPath += pathBox->string( i );
    d.setPath( newPath );
    if ( d.isReadable() ) {
	rereadDir();
    } else {
	d = tmp;
    }
}


/*----------------------------------------------------------------------------
  \internal
  Activated when the "Ok" button is clicked.
 ----------------------------------------------------------------------------*/

void QFileDialog::okClicked()
{
    if ( strcmp( nameEdit->text(), "") != 0 ) {
	emit fileSelected( d.pathName( nameEdit->text() ) );
	accept();
    }
}

/*----------------------------------------------------------------------------
  \internal
  Activated when the "Filter" button is clicked.
 ----------------------------------------------------------------------------*/

void QFileDialog::filterClicked()
{
    if ( strcmp( filterEdit->text(), "" ) == 0 )
	filterEdit->setText( "*" );
    d.setNameFilter( filterEdit->text() );
    rereadDir();
}

/*----------------------------------------------------------------------------
  \internal
  Activated when the "Cancel" button is clicked.
 ----------------------------------------------------------------------------*/

void QFileDialog::cancelClicked()
{
    reject();
}


/*----------------------------------------------------------------------------
  Handles resize events for the file dialog.
 ----------------------------------------------------------------------------*/

void QFileDialog::resizeEvent( QResizeEvent * )
{
    int w = width();
    int h = height();
    int   wTmp;
    QRect rTmp;

    filterL->move( 10, 10 );

    wTmp = filterL->width();
    filterEdit->setGeometry( wTmp + 15, 10, w - wTmp - 15 - 10, 20 );

    rTmp = filterL->geometry();
    wTmp = pathBox->width();
    pathBox->move( (w - wTmp)/2, rTmp.bottom() + 10 );

    rTmp = pathBox->geometry();
    dirL->move( 10, rTmp.bottom() + 5 );

    rTmp = dirL->geometry();
    fileL->move( w / 2 + 5, rTmp.y() );

    rTmp = dirL->geometry();
    dirs->setGeometry(10, rTmp.bottom() + 5, 
		      w/2 - 15, h - rTmp.bottom() - 10 - 25 - 10 - 20 - 10 );

    rTmp = dirs->geometry();
    files->setGeometry( rTmp.right() + 10, rTmp.y(), 
			rTmp.width(), rTmp.height() );

    rTmp = dirs->geometry();
    nameL->move( 10, rTmp.bottom() + 10 );

    wTmp = nameL->width();
    nameEdit->setGeometry( wTmp + 15, rTmp.bottom() + 10, 
			     w - wTmp - 15 - 10, 20 );

    rTmp = nameEdit->geometry();
    okB->move( 10, rTmp.bottom() + 10 );

    rTmp = okB->geometry();
    wTmp = cancelB->width();
    cancelB->move( w - 10 - wTmp, rTmp.y() );

    wTmp = filterB->width();
    filterB->move( (w - wTmp)/2, rTmp.y() );

}


/*----------------------------------------------------------------------------
  \internal
  Updates the path box.  Called from rereadDir().
 ----------------------------------------------------------------------------*/

void QFileDialog::updatePathBox( const char *s )
{
    QStrList l;
    QString tmp;
    QString safe = s;
    
    l.insert( 0, "/" );
    tmp = strtok( safe.data(), "/" );
    while ( TRUE ) {
	if ( tmp.isNull() )
	    break;
	l.insert( 0, tmp + "/" );
	tmp = strtok( 0, "/" );
    }
    pathBox->clear();
    pathBox->insertStrList( &l );
    pathBox->setCurrentItem( 0 );
    pathBox->move( (width() - pathBox->width()) / 2, pathBox->geometry().y() );
}
