/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qfiledialog.cpp#45 $
**
** Implementation of QFileDialog class
**
** Created : 950429
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qfiledlg.h"
#include "qlined.h"
#include "qcombo.h"
#include "qlabel.h"
#include "qpushbt.h"
#include "qmsgbox.h"
#include "qlistview.h"
#include "qapp.h"
#include "qlayout.h"
#include "qlistview.h"

RCSTAG("$Id: //depot/qt/main/src/dialogs/qfiledialog.cpp#45 $");


struct QFileDialogPrivate {
    bool geometryDirty;
    QComboBox * paths;
    QComboBox * types;
    QLabel * pathL;
    QLabel * fileL;
    QLabel * typeL;

    QVBoxLayout * topLevelLayout;

    QPushButton * cdToParent;

    struct File: public QListViewItem {
	File( const QFileInfo * fi, QListViewItem * parent, int h )
	    : info( *fi ), QListViewItem( parent, 0 ) { setHeight( h ); }
	File( const QFileInfo * fi, QListView * parent, int h  )
	    : info( *fi ), QListViewItem( parent, 0 ) { setHeight( h ); }

	void paintCell( QPainter *, const QColorGroup & cg, int, int ) const;

	QFileInfo info;
    };
};


void QFileDialogPrivate::File::paintCell( QPainter * p,
					  const QColorGroup & cg,
					  int c, int w ) const
{
    QString s;
    QRect r( 3, 2, w-6, height()-4 );

    p->fillRect( 0, 0, w, height(), cg.base() );
    p->setPen( cg.text() );

    switch ( c ) {
    case 0:
	p->drawText( r, AlignLeft + AlignVCenter, info.fileName() );
	break;
    case 1:
	if ( info.isFile() ) {
	    s.sprintf( "%d", info.size() );
	    p->drawText( r, AlignRight + AlignVCenter, s );
	}
	break;
    case 2:
	if ( info.isFile() )
	    s = "File";
	else if ( info.isDir() )
	    s = "Directory";
	else
	    s = "Special File";

	if ( info.isSymLink() )
	    s.prepend( "Link to " );

	p->drawText( r, AlignLeft + AlignVCenter, s );
	break;
    case 3:
	p-> drawText( r, AlignLeft + AlignVCenter, 
		      info.lastModified().toString() );
	break;
    case 4:
	p->drawText( r, AlignLeft + AlignVCenter, info.fileName() );
	break;
    }
}

/*!
  \class QFileDialog qfiledlg.h
  \brief The QFileDialog provides a dialog widget for inputting file names.

  Example:
  \code
    QString fileName = QFileDialog::getOpenFileName();
    if ( !fileName.isNull() ) {			// got a file name
	...
    }
  \endcode

  There are two ready-made convenience functions, getOpenFileName()
  and getSaveFileName(), which may be used like this:

  \code
    QString s( QFileDialog::getOpenFileName() );
    if ( s.isNull() )
	return;

    open( s ); // open() being your function to read the file
  \endcode

  <img src=qfiledlg-m.gif> <img src=qfiledlg-w.gif>

  \sa QPrintDialog
*/


/*!
  Constructs a file dialog with a \e parent, \e name and \e modal flag.

  The dialog becomes modal if \e modal is TRUE, otherwise modeless.
*/

QFileDialog::QFileDialog( QWidget *parent, const char *name, bool modal )
    : QDialog( parent, name, modal )
{
    init();
    cwd.convertToAbs();
    rereadDir();
}


/*!
  Constructs a file dialog with a \e parent, \e name and \e modal flag.

  The dialog becomes modal if \e modal is TRUE, otherwise modeless.
*/

QFileDialog::QFileDialog( const char *dirName, const char *filter,
			  QWidget *parent, const char *name, bool modal )
    : QDialog( parent, name, modal )
{
    init();
    if ( filter )
	cwd.setNameFilter( filter );
    if ( dirName )
	cwd.setPath( dirName );
    cwd.convertToAbs();
    rereadDir();
}


/*!
  \internal
  Initializes the file dialog.
*/

void QFileDialog::init()
{
    d = new QFileDialogPrivate();

    nameEdit = new QLineEdit( this, "name/filter editor" );

    files = new QListView( this, "current directory listing" );
    files->setColumn( "Name", 100 );
    files->setColumn( "Size", 50 );
    files->setColumn( "Type", 50 );
    files->setColumn( "Date", 100 );
    files->setColumn( "Attributes", 100 );
    connect( files, SIGNAL(sizeChanged()), SLOT(updateGeometry()) );

    okB = new QPushButton( "OK", this, "OK" );
    okB->setAutoDefault( TRUE );
    okB->setDefault( TRUE );
    connect( okB, SIGNAL(clicked()), this, SLOT(okClicked()) );
    cancelB = new QPushButton( "Cancel" , this, "Cancel" );
    cancelB->setAutoDefault( TRUE );
    connect( cancelB, SIGNAL(clicked()), this, SLOT(cancelClicked()) );

    d->paths = new QComboBox( TRUE, this, "directory history/editor" );
    d->geometryDirty = TRUE;
    d->types = new QComboBox( FALSE, this, "file types" ); // ### TRUE

    d->pathL = new QLabel( d->paths, "Look &in", this );
    d->fileL = new QLabel( nameEdit, "File &name", this );
    d->typeL = new QLabel( d->types, "File &type", this );

    d->cdToParent = new QPushButton( "cd ..", this, "cd to parent" ); // ## pm

    d->topLevelLayout = new QVBoxLayout( this, 6 );

    QHBoxLayout * h;

    h = new QHBoxLayout();
    d->topLevelLayout->addLayout( h );
    h->addWidget( d->pathL );
    h->addWidget( d->paths );
    h->addWidget( d->cdToParent );

    d->topLevelLayout->addWidget( files, 3 );

    h = new QHBoxLayout();
    d->topLevelLayout->addLayout( h );
    h->addWidget( d->fileL );
    h->addWidget( nameEdit );
    h->addWidget( okB );

    h = new QHBoxLayout();
    d->topLevelLayout->addLayout( h );
    h->addWidget( d->typeL );
    h->addWidget( d->types );
    h->addWidget( cancelB );

    cwd.setMatchAllDirs( TRUE );
    cwd.setSorting( cwd.sorting() | QDir::DirsFirst );

    updateGeometry();

    resize( 420, 300 );

    nameEdit->setFocus(); // ### ?
}

/*!
  Destroys the file dialog.
*/

QFileDialog::~QFileDialog()
{
    // nothing
}


/*!
  Returns the selected file name.

  If a file name was selected, the returned string contains the
  absolute path name.  The returned string is a null string if no file
  name was selected.

  \sa QString::isNull()
*/

QString QFileDialog::selectedFile() const
{
    QString tmp;
    if ( nameEdit->text() && strcmp( nameEdit->text(), "" ) != 0 )
	tmp = cwd.absFilePath( nameEdit->text() );
    return tmp;
}


/*!
  Returns the active directory path string in the file dialog.
  \sa dir(), setDir()
*/

const char *QFileDialog::dirPath() const
{
    return cwd.path();
}

/*!
  Sets a directory path string for the file dialog.
  \sa dir()
*/

void QFileDialog::setDir( const char *pathstr )
{
    if ( strcmp(cwd.path(),pathstr) == 0 )
	return;
    cwd.setPath( pathstr );
    cwd.convertToAbs();
    rereadDir();
}

/*!
  Returns the active directory in the file dialog.
  \sa setDir()
*/

const QDir *QFileDialog::dir() const
{
    return &cwd;
}

/*!
  Sets a directory path for the file dialog.
  \sa dir()
*/

void QFileDialog::setDir( const QDir &dir )
{
    cwd = dir;
    cwd.convertToAbs();
    cwd.setMatchAllDirs( TRUE );
    cwd.setSorting( cwd.sorting() | QDir::DirsFirst );
    rereadDir();
}


/*!
  Re-reads the active directory in the file dialog.

  It is seldom necessary to call this function.	 It is provided in
  case the directory contents change and you want to refresh the
  directory list box.
*/

void QFileDialog::rereadDir()
{
    const QFileInfoList *filist = 0;

    int itemHeight = fontMetrics().height() + 6;

    while ( !filist ) {
	filist = cwd.entryInfoList();
	if ( !filist && 
	     QMessageBox::warning( this, "Open File",
				   QString( "Unable to read directory\n" ) +
				   cwd.absPath() + "\n\n"
				   "Please make sure that the directory\n"
				   "in readable.\n",
				   "Use Parent Directory",
				   "Use Old Contents", 0 ) ) {
	    return;
	}
	if ( !filist ) {
	    // change to parent, reread
	    // ...

	    // but for now
	    return;
	}
    }

    files->clear();

    QFileInfoListIterator it( *filist );
    QFileInfo *fi;
    while ( (fi = it.current()) != 0 ) {
	++it;
	(void) new QFileDialogPrivate::File( fi, files, itemHeight );
    }
}



/*!
  \fn void QFileDialog::fileHighlighted( const char * )

  This signal is emitted when the user highlights a file.
*/

/*!
  \fn void QFileDialog::fileSelected( const char * )

  This signal is emitted when the user selects a file.
*/

/*!
  \fn void QFileDialog::dirEntered( const char * )

  This signal is emitted when the user has selected a new directory.
*/

#if defined(_WS_WIN_)
static char *win_filter[] = {
    "All Files", "*.*",
    "" };
#endif

static QString filedlg_dir;


/*!
  Opens a modal file dialog and returns the name of the file to be opened.
  Returns a \link QString::isNull() null string\endlink if the user cancelled
  the dialog.

  This static function is less capable than the full QFileDialog object,
  but is convenient and easy to use.

  Example:
  \code
    // start at the current working directory and with *.cpp as filter
    QString f = QFileDialog::getOpenFileName( 0, "*.cpp", this );
    if ( !f.isEmpty() ) {
        // the user selected a valid existing file
    } else {
        // the user cancelled the dialog
    }
  \endcode
  
  getSaveFileName() is another convenience function, equal to this one
  except that it allows the user to specify the name of a nonexistent file
  name.

  \sa getSaveFileName()
*/

QString QFileDialog::getOpenFileName( const char *dirName, const char *filter,
				      QWidget *parent, const char *name )
{
    if ( dirName && *dirName ) {
	filedlg_dir = dirName;
    } else if ( filedlg_dir.isNull() ) {
	filedlg_dir = QDir::currentDirPath();
    }

#if defined(_WS_WIN_)

    filedlg_dir = QDir::convertSeparators( filedlg_dir );

    const int maxstrlen = 256;
    char *file = new char[maxstrlen];
    file[0] = '\0';

    OPENFILENAME ofn;
    memset( &ofn, 0, sizeof(OPENFILENAME) );
    ofn.lStructSize	= sizeof(OPENFILENAME);
    ofn.hwndOwner	= parent ? parent->topLevelWidget()->winId() : 0;
    ofn.lpstrFilter	= win_filter[0];
    ofn.lpstrFile	= file;
    ofn.nMaxFile	= maxstrlen;
    ofn.lpstrInitialDir = filedlg_dir;
    ofn.lpstrTitle	= "Open";
    ofn.Flags		= (OFN_CREATEPROMPT|OFN_NOCHANGEDIR);

    QString result;
    if ( GetOpenFileName(&ofn) ) {
	result = file;
	filedlg_dir = QFileInfo(file).dirPath();
    }

    delete [] file;
    return result;

#else

    QFileDialog *dlg = new QFileDialog( filedlg_dir, filter, parent, name, TRUE );
    CHECK_PTR( dlg );
    dlg->setCaption( "Open" );
    QString result;
    if ( dlg->exec() == QDialog::Accepted ) {
	result = dlg->selectedFile();
	filedlg_dir = dlg->dirPath();
    }
    delete dlg;
    return result;

#endif
}

/*!
  Opens a modal file dialog and returns the name of the file to be saved.
  Returns a \link QString::isNull() null string\endlink if the user cancelled
  the dialog.

  This static function is less capable than the full QFileDialog object,
  but is convenient and easy to use.

  Example:
  \code
    // start at the current working directory and with *.cpp as filter
    QString f = QFileDialog::getSaveFileName( 0, "*.cpp", this );
    if ( !f.isEmpty() ) {
        // the user gave a file name
    } else {
        // the user cancelled the dialog
    }
  \endcode
  
  getOpenFileName() is another convenience function, equal to this one
  except that it allows the user to specify the name of a nonexistent file
  name.

  \sa getOpenFileName()
*/

QString QFileDialog::getSaveFileName( const char *dirName, const char *filter,
				      QWidget *parent, const char *name )
{
    if ( dirName && *dirName ) {
	filedlg_dir = dirName;
    } else if ( filedlg_dir.isNull() ) {
	filedlg_dir = QDir::currentDirPath();
    }

#if defined(_WS_WIN_)

    filedlg_dir = QDir::convertSeparators( filedlg_dir );

    const int maxstrlen = 256;
    char *file = new char[maxstrlen];
    file[0] = '\0';

    OPENFILENAME ofn;
    memset( &ofn, 0, sizeof(OPENFILENAME) );
    ofn.lStructSize	= sizeof(OPENFILENAME);
    ofn.hwndOwner	= parent ? parent->topLevelWidget()->winId() : 0;
    ofn.lpstrFilter	= win_filter[0];
    ofn.lpstrFile	= file;
    ofn.nMaxFile	= maxstrlen;
    ofn.lpstrInitialDir = filedlg_dir;
    ofn.lpstrTitle	= "Save";
    ofn.Flags		= (OFN_CREATEPROMPT|OFN_NOCHANGEDIR);

    QString result;
    if ( GetSaveFileName(&ofn) ) {
	result = file;
	filedlg_dir = QFileInfo(file).dirPath();
    }

    delete [] file;
    return result;

#else

    QFileDialog *dlg = new QFileDialog( filedlg_dir, filter, parent, name, TRUE );
    CHECK_PTR( dlg );
    dlg->setCaption( "Save As" );
    QString result;
    if ( dlg->exec() == QDialog::Accepted ) {
	result = dlg->selectedFile();
	filedlg_dir = dlg->dirPath();
    }
    delete dlg;
    return result;

#endif
}


/*!
  \internal
  Activated when a file name in the file list has been selected.
*/

void QFileDialog::fileSelected( int  )
{
    nameEdit->setText( "sex" );
    emit fileSelected( cwd.filePath( nameEdit->text() ) );
    accept();
}

/*!
  \internal
  Activated when a file name in the file list has been highlighted.
*/

void QFileDialog::fileHighlighted( int )
{
    // unused
}

/*!
  \internal
  Activated when a directory name in the directory list has been selected.
*/

void QFileDialog::dirSelected( int )
{
    // unused
}

void QFileDialog::pathSelected( int )
{
    // unused
}


/*!
  \internal
  Activated when the "Ok" button is clicked.
*/

void QFileDialog::okClicked()
{
    if ( strcmp( nameEdit->text(), "") != 0 ) {
	emit fileSelected( cwd.filePath( nameEdit->text() ) );
	accept();
    }
}

/*!
  \internal
  Activated when the "Filter" button is clicked.
*/

void QFileDialog::filterClicked()
{
    // unused
}

/*!
  \internal
  Activated when the "Cancel" button is clicked.
*/

void QFileDialog::cancelClicked()
{
    reject();
}


/*!
  Handles resize events for the file dialog.
*/

void QFileDialog::resizeEvent( QResizeEvent * )
{
    updateGeometry();
}

/*! \internal 

  Obsolete.
*/
void QFileDialog::updatePathBox( const char * )
{
    // unused
}


/*!  Make sure the minimum and maximum sizes of everything are sane.
*/

void QFileDialog::updateGeometry()
{
    if ( !d || !d->geometryDirty )
	return;

    d->geometryDirty = FALSE;

    QSize r, t;

    // we really should have a QSize::unite()
#define RM r.setWidth( QMAX(r.width(),t.width()) ); \
    r.setHeight( QMAX(r.height(),t.height()) )

    // labels first
    r = d->pathL->sizeHint();
    t = d->fileL->sizeHint();
    RM;
    t = d->typeL->sizeHint();
    RM;
    d->pathL->setFixedSize( r );
    d->fileL->setFixedSize( r );
    d->typeL->setFixedSize( r );

    // single-line input areas
    r = d->paths->sizeHint();
    t = nameEdit->sizeHint();
    RM;
    t = d->types->sizeHint();
    RM;
    t.setWidth( QCOORD_MAX );
    t.setHeight( r.height() );
    d->paths->setMinimumSize( r );
    d->paths->setMaximumSize( t );
    nameEdit->setMinimumSize( r );
    nameEdit->setMaximumSize( t );
    d->types->setMinimumSize( r );
    d->types->setMaximumSize( t );

    // buttons on top row 
    r = QSize( 0, d->paths->minimumSize().height() );
    t = d->cdToParent->sizeHint();
    RM;
    // ...
    d->cdToParent->setFixedSize( r );
    // ...

    // open/save, cancel
    r = QSize( 80, 0 );
    t = okB->sizeHint();
    RM;
    t = cancelB->sizeHint();
    RM;
    okB->setFixedSize( r );
    cancelB->setFixedSize( r );

    d->topLevelLayout->activate();

#undef RM
}
