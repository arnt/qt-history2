/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qfiledialog.cpp#158 $
**
** Implementation of QFileDialog class
**
** Created : 950429
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qfiledialog.h"
#include "qlineedit.h"
#include "qcombobox.h"
#include "qlabel.h"
#include "qpushbutton.h"
#include "qmessagebox.h"
#include "qlistview.h"
#include "qlistbox.h"
#include "qapplication.h"
#include "qlayout.h"
#include "qbitmap.h"
#include "qpopupmenu.h"
#include "qwidgetstack.h"
#include "qbuttongroup.h"
#include "qvector.h"
#include "qregexp.h"
#include "qstrlist.h"
#include "qtimer.h"

#include <time.h>
#include <ctype.h>

#if defined(UNIX)
// getlogin()
#include <unistd.h>

// getpwnam()
#include <sys/types.h>
#include <pwd.h>
#endif

// see comment near use of this variable
static const char * egcsWorkaround = "%x  %X";

static QFileIconProvider * fileIconProvider = 0;


/* XPM */
static const char* open_xpm[]={
"16 13 6 1",
". c None",
"b c #ffff00",
"d c #000000",
"# c #999999",
"c c #cccccc",
"a c #ffffff",
"...#####........",
"..#aaaaa#.......",
".#abcbcba######.",
".#acbcbcaaaaaa#d",
".#abcbcbcbcbcb#d",
"#############b#d",
"#aaaaaaaaaa##c#d",
"#abcbcbcbcbbd##d",
".#abcbcbcbcbcd#d",
".#acbcbcbcbcbd#d",
"..#acbcbcbcbb#dd",
"..#############d",
"...ddddddddddddd"};
/* XPM */
static const char* closed_xpm[]={
"15 13 6 1",
". c None",
"b c #ffff00",
"d c #000000",
"# c #999999",
"a c #cccccc",
"c c #ffffff",
"..#####........",
".#ababa#.......",
"#abababa######.",
"#cccccccccccc#d",
"#cbababababab#d",
"#cabababababa#d",
"#cbababababab#d",
"#cabababababa#d",
"#cbababababab#d",
"#cabababababa#d",
"#cbababababab#d",
"##############d",
".dddddddddddddd"};


/* XPM */
static const char* cdtoparent_xpm[]={
"15 13 3 1",
". c None",
"# c #000000",
"a c #ffff99",
"..#####........",
".#aaaaa#.......",
"###############",
"#aaaaaaaaaaaaa#",
"#aaaa#aaaaaaaa#",
"#aaa###aaaaaaa#",
"#aa#####aaaaaa#",
"#aaaa#aaaaaaaa#",
"#aaaa#aaaaaaaa#",
"#aaaa######aaa#",
"#aaaaaaaaaaaaa#",
"#aaaaaaaaaaaaa#",
"###############"};


/* XPM */
static const char* detailedview_xpm[]={
"14 11 3 1",
". c None",
"# c #000000",
"a c #000099",
".####.###.###.",
"..............",
"aaaaaaaaaaaaaa",
"..............",
".####.###.###.",
"..............",
".####.###.###.",
"..............",
".####.###.###.",
"..............",
".####.###.###."};


/* XPM */
static const char* mclistview_xpm[]={
"15 11 4 1",
"# c None",
"b c #000000",
". c #000099",
"a c #ffffff",
"...#####...####",
".a.#bbb#.a.#bbb",
"...#####...####",
"###############",
"...#####...####",
".a.#bbb#.a.#bbb",
"...#####...####",
"###############",
"...#####...####",
".a.#bbb#.a.#bbb",
"...#####...####"};


static QPixmap * openFolderIcon = 0;
static QPixmap * closedFolderIcon = 0;
static QPixmap * detailViewIcon = 0;
static QPixmap * multiColumnListViewIcon = 0;
static QPixmap * cdToParentIcon = 0;
static QPixmap * fifteenTransparentPixels = 0;

static QString * tmpString = 0;
static QString * workingDirectory = 0;

static void cleanup() {
    delete openFolderIcon;
    openFolderIcon = 0;
    delete closedFolderIcon;
    closedFolderIcon = 0;
    delete detailViewIcon;
    detailViewIcon = 0;
    delete multiColumnListViewIcon;
    multiColumnListViewIcon = 0;
    delete cdToParentIcon;
    cdToParentIcon = 0;
    delete fifteenTransparentPixels;
    fifteenTransparentPixels = 0;
    delete tmpString;
    tmpString = 0;
    delete workingDirectory;
    workingDirectory = 0;
}


static void makeVariables() {
    if ( !tmpString ) {
	qAddPostRoutine( cleanup );
	tmpString = new QString;
	workingDirectory = new QString;
	openFolderIcon = new QPixmap(open_xpm);
	closedFolderIcon = new QPixmap(closed_xpm);
	detailViewIcon = new QPixmap(detailedview_xpm);
	multiColumnListViewIcon = new QPixmap(mclistview_xpm);
	cdToParentIcon = new QPixmap(cdtoparent_xpm);
	fifteenTransparentPixels = new QPixmap( closedFolderIcon->width(), 1 );
	QBitmap m( fifteenTransparentPixels->width(), 1 );
	m.fill( Qt::color0 );
	fifteenTransparentPixels->setMask( m );
    }
}


struct QFileDialogPrivate {
    bool geometryDirty;
    QComboBox * paths;
    QComboBox * types;
    QLabel * pathL;
    QLabel * fileL;
    QLabel * typeL;

    QVBoxLayout * topLevelLayout;
    QHBoxLayout * extraWidgetsLayout;
    QLabel * extraLabel;
    QWidget * extraWidget;
    QButton * extraButton;

    QWidgetStack * stack;

    QPushButton * cdToParent, * detailView, * mcView;
    QButtonGroup * modeButtons;

    QString currentFileName;

    struct File: public QListViewItem {
	File( QFileDialogPrivate * dlgp,
	      const QFileInfo * fi, QListViewItem * parent )
	    : QListViewItem( parent ), info( *fi ), d(dlgp) { setup(); }
	File( QFileDialogPrivate * dlgp,
	      const QFileInfo * fi, QListView * parent )
	    : QListViewItem( parent ), info( *fi ), d(dlgp) { setup(); }

	QString text( int column ) const;
	QString key( int column, bool ) const;
	const QPixmap * pixmap( int ) const;

	QFileInfo info;
	QFileDialogPrivate * d;
    };

    class MCItem: public QListBoxItem {
    public:
	MCItem( QListBox *, QListViewItem * item );
	QString text() const;
	const QPixmap *pixmap() const;
	int height( const QListBox * ) const;
	int width( const QListBox * ) const;
	void paint( QPainter * );
	QListViewItem * i;
    };

    QListBox * moreFiles;

    QFileDialog::Mode mode;

    QString rw;
    QString ro;
    QString wo;
    QString inaccessible;

    QString symLinkToFile;
    QString file;
    QString symLinkToDir;
    QString dir;
    QString symLinkToSpecial;
    QString special;
};


QString QFileDialogPrivate::File::text( int column ) const
{
    makeVariables();

    switch( column ) {
    case 0:
	*tmpString = info.fileName();
	break;
    case 1:
	if ( info.isFile() )
	    tmpString->sprintf( "%d", info.size() );
	else
	    *tmpString = "";
	break;
    case 2:
	if ( info.isFile() && info.isSymLink() )
	    *tmpString = d->symLinkToFile;
	else if ( info.isFile() )
	    *tmpString = d->file;
	else if ( info.isDir() && info.isSymLink() )
	    *tmpString = d->symLinkToDir;
	else if ( info.isDir() )
	    *tmpString = d->dir;
	else if ( info.isSymLink() )
	    *tmpString = d->symLinkToSpecial;
	else
	    *tmpString = d->special;
	break;
    case 3:
	{
	    QDateTime epoch;
	    epoch.setTime_t( 0 );
	    char a[256];
	    time_t t1 = epoch.secsTo( info.lastModified() );
	    struct tm * t2 = ::localtime( &t1 );
	    // use a static const char here, so that egcs will not see
	    // the formatting string and give an incorrect warning.
	    if ( strftime( a, 255, egcsWorkaround, t2 ) > 0 )
		*tmpString = a;
	    else
		*tmpString = "????";
	}
	break;
    case 4:
	if ( info.isReadable() )
	    *tmpString = info.isWritable() ? d->rw : d->ro;
	else
	    *tmpString = info.isWritable() ? d->wo : d->inaccessible;
	break;
    default:
	*tmpString = "<--->";
    }

    return *tmpString;
}


const QPixmap * QFileDialogPrivate::File::pixmap( int column ) const
{
    if ( column )
	return 0;
    else if ( fileIconProvider )
	return fileIconProvider->pixmap( info );
    else if ( info.isDir() )
	return closedFolderIcon;
    else
	return fifteenTransparentPixels;
}


QString QFileDialogPrivate::File::key( int column, bool ascending ) const
{
    makeVariables();
    QDateTime epoch( QDate( 1968, 6, 19 ) );

    char majorkey = ascending == info.isDir() ? '0' : '1';

    if ( info.fileName() == ".." ) {
        *tmpString = ascending ? "0" : "a"; // a > 9
    } else if ( column == 1 ) {
	tmpString->sprintf( "%c%08d", majorkey, info.size() );
    } else if ( column == 3 ) {
	tmpString->sprintf( "%c%08d",
			    majorkey, epoch.secsTo( info.lastModified() ) );
    } else {
	*tmpString = text( column );
	tmpString->insert( 0, majorkey );
    }

    return *tmpString;
}


QFileDialogPrivate::MCItem::MCItem( QListBox * lb, QListViewItem * item )
    : QListBoxItem()
{
    i = item;
    lb->insertItem( this );
}


QString QFileDialogPrivate::MCItem::text() const
{
    return i->text( 0 );
}


const QPixmap *QFileDialogPrivate::MCItem::pixmap() const
{
    return i->pixmap( 0 );
}


int QFileDialogPrivate::MCItem::height( const QListBox * lb ) const
{
    return lb->fontMetrics().height() + 4;
}


int QFileDialogPrivate::MCItem::width( const QListBox * lb ) const
{
    const QFontMetrics & fm = lb->fontMetrics();
    int w = 4;
    if ( pixmap() )
	w += pixmap()->width();
    w += fm.width( text() );
    return w;
}


void QFileDialogPrivate::MCItem::paint( QPainter * p )
{
    const QPixmap * pm = pixmap();
    if ( pm )
	p->drawPixmap( 2, 2, *pm );
    QFontMetrics fm = p->fontMetrics();
    int yPos;			// vertical text position
    if ( !pm || pm->height() < fm.height() )
	yPos = fm.ascent() + fm.leading()/2;
    else
	yPos = pm->height()/2 - fm.height()/2 + fm.ascent();
    p->drawText( pm ? pm->width()+4 : 20, yPos, text() );
}


/*!
  \class QFileDialog qfiledialog.h
  \brief The QFileDialog provides a dialog widget for inputting file names.
  \ingroup dialogs

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

QFileDialog::QFileDialog( const QString& dirName, const QString & filter,
			  QWidget *parent, const char *name, bool modal )
    : QDialog( parent, name, modal )
{
    init();
    if ( !filter.isEmpty() ) {
	cwd.setNameFilter( filter );
	d->types->insertItem( filter );
    } else {
	d->types->insertItem( tr( "All files (*)" ) );
    }
    if ( !dirName.isEmpty() )
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
    d->mode = AnyFile;

    nameEdit = new QLineEdit( this, "name/filter editor" );
    connect( nameEdit, SIGNAL(textChanged(const QString&)),
	     this,  SLOT(fileNameEditDone()) );
    nameEdit->installEventFilter( this );

    d->stack = new QWidgetStack( this, "files and more files" );
    d->stack->setFrameStyle( QFrame::WinPanel + QFrame::Sunken );
			
    files = new QListView( d->stack, "current directory listing" );
    QFontMetrics fm = fontMetrics();
    files->addColumn( tr("Name"), 150 );
    files->setColumnWidthMode( 0, QListView::Manual );
    files->addColumn( tr("Size"), 30 + fm.width( tr("Size") ) );
    files->setColumnWidthMode( 1, QListView::Maximum );
    files->setColumnAlignment( 1, AlignRight );
    files->addColumn( tr("Type"), 10 + fm.width( tr("Directory") ) );
    files->setColumnWidthMode( 2, QListView::Maximum );
    files->addColumn( tr("Date"), 50 );
    files->setColumnWidthMode( 3, QListView::Maximum );
    files->addColumn( tr("Attributes"), 10 + fm.width( tr("Attributes") ) );
    files->setColumnWidthMode( 0, QListView::Maximum );

    files->setMinimumSize( 50, 25 + 2*fontMetrics().lineSpacing() );

    connect( files, SIGNAL(selectionChanged(QListViewItem *)),
	     this, SLOT(updateFileNameEdit(QListViewItem *)) );
    connect( files, SIGNAL(doubleClicked(QListViewItem *)),
	     this, SLOT(selectDirectoryOrFile(QListViewItem *)) );
    connect( files, SIGNAL(returnPressed(QListViewItem *)),
	     this, SLOT(selectDirectoryOrFile(QListViewItem *)) );
    connect( files, SIGNAL(rightButtonClicked(QListViewItem *,
					      const QPoint &, int)),
	     this, SLOT(popupContextMenu(QListViewItem *,
					 const QPoint &, int)) );

    files->setFocusPolicy( StrongFocus );

    files->installEventFilter( this );
    files->viewport()->installEventFilter( this );

    d->moreFiles = new QListBox( d->stack );
    d->moreFiles->setFrameStyle( QFrame::NoFrame );
    d->moreFiles->setFocusPolicy( StrongFocus );
    d->moreFiles->setRowMode( QListBox::FitToHeight );
    d->moreFiles->setVariableWidth( TRUE );

    connect( d->moreFiles, SIGNAL(highlighted(QListBoxItem *)),
	     this, SLOT(updateFileNameEdit(QListBoxItem *)) );
    connect( d->moreFiles, SIGNAL(selected(QListBoxItem *)),
	     this, SLOT(selectDirectoryOrFile(QListBoxItem *)) );

    okB = new QPushButton( tr("OK"), this, "OK" ); //### Or "Save (see other "OK")
    okB->setAutoDefault( TRUE );
    okB->setDefault( TRUE );
    okB->setEnabled( FALSE );
    connect( okB, SIGNAL(clicked()), this, SLOT(okClicked()) );
    cancelB = new QPushButton( tr("Cancel") , this, "Cancel" );
    cancelB->setAutoDefault( TRUE );
    connect( cancelB, SIGNAL(clicked()), this, SLOT(cancelClicked()) );

    d->paths = new QComboBox( TRUE, this, "directory history/editor" );
    const QFileInfoList * rootDrives = QDir::drives();
    QFileInfoListIterator it( *rootDrives );
    QFileInfo *fi;
    while ( (fi = it.current()) != 0 ) {
	++it;
	d->paths->insertItem( fi->absFilePath() );
    }
    connect( d->paths, SIGNAL(activated(const QString&)),
	     this, SLOT(setDir(const QString&)) );

    d->geometryDirty = TRUE;
    d->types = new QComboBox( TRUE, this, "file types" );
    connect( d->types, SIGNAL(activated(const QString&)),
	     this, SLOT(setFilter(const QString&)) );

    d->pathL = new QLabel( d->paths, tr("Look &in"), this );
    d->fileL = new QLabel( nameEdit, tr("File &name"), this );
    d->typeL = new QLabel( d->types, tr("File &type"), this );

    makeVariables();

    d->cdToParent = new QPushButton( this, "cd to parent" );
    d->cdToParent->setPixmap( *cdToParentIcon );
    connect( d->cdToParent, SIGNAL(clicked()),
	     this, SLOT(cdUpClicked()) );

    d->modeButtons = new QButtonGroup( 0, "invisible group" );
    connect( d->modeButtons, SIGNAL(destroyed()),
	     this, SLOT(modeButtonsDestroyed()) );
    d->modeButtons->setExclusive( TRUE );
    connect( d->modeButtons, SIGNAL(clicked(int)),
	     d->stack, SLOT(raiseWidget(int)) );

    d->detailView = new QPushButton( this, "list view" );
    d->detailView->setPixmap( *detailViewIcon );
    d->detailView->setToggleButton( TRUE );
    d->stack->addWidget( files, d->modeButtons->insert( d->detailView ) );
    d->mcView = new QPushButton( this, "mclistbox view" );
    d->mcView->setPixmap( *multiColumnListViewIcon );
    d->mcView->setToggleButton( TRUE );
    d->stack->addWidget( d->moreFiles, d->modeButtons->insert( d->mcView ) );

    d->stack->raiseWidget( d->moreFiles );
    d->mcView->setOn( TRUE );

    d->topLevelLayout = new QVBoxLayout( this, 5 );
    d->extraWidgetsLayout = 0;
    d->extraLabel = 0;
    d->extraWidget = 0;
    d->extraButton = 0;

    QHBoxLayout * h;

    h = new QHBoxLayout( 0 );
    d->topLevelLayout->addLayout( h );
    h->addWidget( d->pathL );
    h->addSpacing( 8 );
    h->addWidget( d->paths );
    h->addSpacing( 8 );
    h->addWidget( d->cdToParent );
    h->addSpacing( 8 );
    h->addWidget( d->detailView );
    h->addWidget( d->mcView );
    h->addSpacing( 16 );

    d->topLevelLayout->addWidget( d->stack, 3 );

    h = new QHBoxLayout();
    d->topLevelLayout->addLayout( h );
    h->addWidget( d->fileL );
    h->addWidget( nameEdit );
    h->addSpacing( 15 );
    h->addWidget( okB );

    h = new QHBoxLayout();
    d->topLevelLayout->addLayout( h );
    h->addWidget( d->typeL );
    h->addWidget( d->types );
    h->addSpacing( 15 );
    h->addWidget( cancelB );

    cwd.setMatchAllDirs( TRUE );
    cwd.setSorting( cwd.sorting() );

    updateGeometry();

    d->cdToParent->setFocusPolicy( NoFocus );
    d->detailView->setFocusPolicy( NoFocus );
    d->mcView->setFocusPolicy( NoFocus );

    setTabOrder( d->paths, d->moreFiles );
    setTabOrder( d->moreFiles, files );
    setTabOrder( files, nameEdit );
    setTabOrder( nameEdit, d->types );
    setTabOrder( d->types, okB );
    setTabOrder( okB, cancelB );

    nameEdit->setFocus();

    setFontPropagation( SameFont );
    setPalettePropagation( SamePalette );

    d->rw = tr( "Read-write" );
    d->ro = tr( "Read-only" );
    d->wo = tr( "Write-only" );
    d->inaccessible = tr( "Inaccessible" );

    d->symLinkToFile = tr( "Symlink to File" );
    d->symLinkToDir = tr( "Symlink to Directory" );
    d->symLinkToSpecial = tr( "Symlink to Special" );
    d->file = tr( "File" );
    d->dir = tr( "Dir" );
    d->special = tr( "Special" );

    if ( QApplication::desktop()->width() < 1024 ||
	 QApplication::desktop()->height() < 768 ) {
	resize( 420, 236 );
    } else {
	QSize s( files->sizeHint() );
	s = QSize( s.width() + 250, s.height() + 82 );

	if ( s.width() * 3 > QApplication::desktop()->width() * 2 )
	    s.setWidth( QApplication::desktop()->width() * 2 / 3 );

	if ( s.height() * 3 > QApplication::desktop()->height() * 2 )
	    s.setHeight( QApplication::desktop()->height() * 2 / 3 );
	else if ( s.height() * 3 < QApplication::desktop()->height() )
	    s.setHeight( QApplication::desktop()->height() / 3 );

	resize( s );
    }
}

/*!
  Destroys the file dialog.
*/

QFileDialog::~QFileDialog()
{
    delete d->modeButtons;
    delete d;
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
    return d->currentFileName;
}

/*!
  Sets the default selection to \a filename.  If \a filename is
  absolute, setDir() is also called.

  \internal
  Only for external use.  Not useful inside QFileDialog.
*/
void QFileDialog::setSelection( const QString & filename )
{
    QFileInfo info(filename);
    if ( info.isDir() ) {
	setDir( filename );
	nameEdit->setText( "" );
    } else {
	setDir( info.dir() );
	nameEdit->setText( info.fileName() );
    }
    trySetSelection( info, FALSE );
}

/*!
  Returns the active directory path string in the file dialog.
  \sa dir(), setDir()
*/

QString QFileDialog::dirPath() const
{
    return cwd.path();
}


/*!  Sets the filter spec in use to \a newFilter.

  If \a newFilter matches the regular expression
  <tt>([a-zA-Z0-9\.\*\?]*)$</tt> (ie. it ends with a normal wildcard
  expression enclosed in parentheses), only the parenthesized is used.
  This means that these two calls are equivalent:

  \code
     fd->setFilter( "All perl files (*.pl)" );
     fd->setFilter( "*.pl" )
  \endcode
*/

void QFileDialog::setFilter( const QString & newFilter )
{
    if ( !newFilter )
	return;
    QString f = newFilter;
    QRegExp r( "([a-zA-Z0-9\\.\\*\\?]*)$" );
    int len;
    int index = r.match( f, 0, &len );
    if ( index >= 0 )
	f = f.mid( index+1, len-2 );
    cwd.setNameFilter( f );
    rereadDir();
}


/*!
  Sets a directory path string for the file dialog.
  \sa dir()
*/

void QFileDialog::setDir( const QString & pathstr )
{
    QString d = pathstr;

#if defined(UNIX)
    if ( d.length() && d[0] == '~' ) {
	struct passwd *pw;
	int i;

	i = 0;
	while( i < (int)d.length() && d[i] != '/' )
	    i++;
	QString user;
	user = ( i == 1 ) ? QString( ::getlogin() ) : d.mid( 1, i-1 );
	d = d.mid( i, d.length() );
	pw = ::getpwnam( user );
	if ( pw )
	    d.prepend( pw->pw_dir );
    }
#endif

    QDir tmp( d );
    tmp.setFilter( cwd.filter() );
    setDir( tmp );
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
    if ( !dir.exists() ||
	 dir.absPath() == cwd.absPath() )
	return;
    QString nf( cwd.nameFilter() );
    cwd = dir;
    cwd.setNameFilter( nf );
    cwd.convertToAbs();
    cwd.setMatchAllDirs( TRUE );
    cwd.setSorting( cwd.sorting() );
    QFileInfo i( cwd, nameEdit->text() );
    trySetSelection( i, FALSE );
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
    if ( d ) {
	QString cp( cwd.canonicalPath() );
	int i = d->paths->count()-1;
	while( i >= 0 && strcmp( d->paths->text( i ), cp ) >= 0 )
	    i--;
	if ( i < d->paths->count() )
	    i++;
	if ( i == d->paths->count() || strcmp( d->paths->text( i ), cp ) )
	    d->paths->insertItem( cwd.canonicalPath(), i );
	d->paths->setCurrentItem( i );
    }

    d->cdToParent->setEnabled( !cwd.isRoot() );

    const QFileInfoList *filist = 0;

    while ( !filist ) {
	filist = cwd.entryInfoList();
	if ( !filist &&
	     QMessageBox::warning( this, tr("Open File"),
				   QString( tr("Unable to read directory\n") )
				   + cwd.absPath() + "\n\n" +
				   tr("Please make sure that the directory\n"
				      "is readable.\n"),
				   tr("Use Parent Directory"),
				   tr("Use Old Contents") ) ) {
	    return;
	}
	if ( !filist ) {
	    QString tmp( cwd.absPath() );
	
	    // change to parent, reread
	    // ...

	    // but for now
	    return;
	}
    }

    d->moreFiles->clear();
    files->clear();

    QFileInfoListIterator it( *filist );
    QFileInfo *fi;
    while ( (fi = it.current()) != 0 ) {
	++it;
	if ( fi->fileName() != "." &&
	     ( !cwd.isRoot() || fi->fileName() != ".." ) ) {
	    QListViewItem * i
		= new QFileDialogPrivate::File( d, fi, files );
	    if ( mode() == ExistingFiles && fi->isDir() )
		i->setSelectable( FALSE );
	    (void)new QFileDialogPrivate::MCItem( d->moreFiles, i );
	}
    }
}


/*!
  \fn void QFileDialog::fileHighlighted( const QString& )

  This signal is emitted when the user highlights a file.
*/

/*!
  \fn void QFileDialog::fileSelected( const QString& )

  This signal is emitted when the user selects a file.
*/

/*!
  \fn void QFileDialog::dirEntered( const QString& )

  This signal is emitted when the user has selected a new directory.
*/

// Defined in qapplication.cpp:
void qt_enter_modal( QWidget* );
void qt_leave_modal( QWidget* );

/*!
  Opens a modal file dialog and returns the name of the file to be
  opened.

  If \a startWith is the name of a directory, the dialog starts off in
  that directory.  If \a startWith is the name of an existing file,
  the dialogs starts in that directory, and with \a startWith
  selected.

  Only files matching \a filter are selectable.  If \a filter is 0,
  all files are selectable.

  If \a widget and/or \a name is provided, the dialog will be centered
  over \a widget and \link QObject::name() named \endlink \a name.

  getOpenFileName() returns a \link QString::isNull() null string
  \endlink if the user cancelled the dialog.

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

QString QFileDialog::getOpenFileName( const QString & startWith,
				      const QString& filter,
				      QWidget *parent, const char* name )
{
    makeVariables();
    QString initialSelection;
    if ( !startWith.isEmpty() ) {
	QFileInfo fi( startWith );
	if ( fi.exists() && fi.isDir() ) {
	    *workingDirectory = startWith;
	} else if ( fi.exists() && fi.isFile() ) {
	    *workingDirectory = fi.dirPath( TRUE );
	    initialSelection = fi.absFilePath();
	}
    }

    if ( workingDirectory->isNull() )
	*workingDirectory = QDir::currentDirPath();

    QFileDialog *dlg = new QFileDialog( *workingDirectory, filter,
					parent, name, TRUE );
    CHECK_PTR( dlg );
    dlg->setCaption( "Open" );
    if ( !initialSelection.isEmpty() )
	dlg->setSelection( initialSelection );
    dlg->setMode( QFileDialog::ExistingFile );
    QString result;
    if ( dlg->exec() == QDialog::Accepted ) {
	result = dlg->selectedFile();
	*workingDirectory = dlg->dirPath();
    }
    delete dlg;
    return result;
}

/*!
  Opens a modal file dialog and returns the name of the file to be
  saved.

  If \a startWith is the name of a directory, the dialog starts off in
  that directory.  If \a startWith is the name of an existing file,
  the dialogs starts in that directory, and with \a startWith
  selected.

  Only files matching \a filter are selectable.  If \a filter is 0,
  all files are selectable.

  If \a widget and/or \a name is provided, the dialog will be centered
  over \a widget and \link QObject::name() named \endlink \a name.

  Returns a \link QString::isNull() null string\endlink if the user
  cancelled the dialog.

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
  except that it does not allow the user to specify the name of a
  nonexistent file name.

  \sa getOpenFileName()
*/

QString QFileDialog::getSaveFileName( const QString & startWith,
				      const QString& filter,
				      QWidget *parent, const char* name )
{
    makeVariables();
    QString initialSelection;
    if ( !startWith.isEmpty() ) {
	QFileInfo fi( startWith );
	if ( fi.exists() && fi.isDir() ) {
	    *workingDirectory = startWith;
	} else if ( !fi.exists() || fi.isFile() ) {
	    *workingDirectory = fi.dirPath( TRUE );
	    initialSelection = fi.absFilePath();
	}
    }

    if ( workingDirectory->isNull() )
	*workingDirectory = QDir::currentDirPath();

    QFileDialog *dlg = new QFileDialog( *workingDirectory, filter, parent, name, TRUE );
    CHECK_PTR( dlg );
    dlg->setCaption( "Save As" );
    QString result;
    if ( !initialSelection.isEmpty() )
	dlg->setSelection( initialSelection );
    if ( dlg->exec() == QDialog::Accepted ) {
	result = dlg->selectedFile();
	*workingDirectory = dlg->dirPath();
    }
    delete dlg;
    return result;
}


/*!
  \internal
  Activated when the "Ok" button is clicked.
*/

void QFileDialog::okClicked()
{
    // if we're in multi-selection mode and something is selected,
    // accept it and be done.
    if ( mode() == ExistingFiles ) {
	QListViewItem * i = files->firstChild();
	while( i && !i->isSelected() )
	    i = i->nextSibling();
	if ( i )
	    accept();
    }

    // If selection is valid, return it, else try
    // using selection as a directory to change to.
    if ( !d->currentFileName.isNull() ) {
	emit fileSelected( d->currentFileName );
	accept();
    } else {
	QFileInfo f;
	QFileDialogPrivate::File * c
	    = (QFileDialogPrivate::File *)files->currentItem();
        if ( c && files->isSelected(c) )
	    f = c->info;
	else
	    f = QFileInfo( cwd, nameEdit->text() );
	if ( f.isDir() ) {
	    setDir( f.absFilePath() );
	    QFileInfo f ( cwd, "." );
	    trySetSelection( f, TRUE );
	}
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

/*
  \internal
  The only correct way to try to set currentFileName
*/
bool QFileDialog::trySetSelection( const QFileInfo& info, bool updatelined )
{
    QString old = d->currentFileName;

    if ( mode() == Directory ) {
	if ( info.isDir() )
	    d->currentFileName = info.absFilePath();
	else
	    d->currentFileName = 0;
    } else if ( info.isFile() && mode() == ExistingFiles ) {
	d->currentFileName = info.absFilePath();
    } else if ( info.isFile() || (mode() == AnyFile && !info.isDir()) ) {
	d->currentFileName = info.absFilePath();
    } else {
	d->currentFileName = 0;
    }
    if ( updatelined && !d->currentFileName.isNull() ) {
	// If the selection is valid, or if its a directory, allow OK.
	if ( !d->currentFileName.isNull() || info.isDir() )
	    nameEdit->setText( info.fileName() );
	else
	    nameEdit->setText( "" );
    }

    if ( !d->currentFileName.isNull() || info.isDir() ) {
	okB->setEnabled( TRUE );
	if ( d->currentFileName.isNull() && info.isDir() )
	    okB->setText(tr("Open"));
	else
	    okB->setText( mode() == AnyFile ? tr("Save") : tr("OK"));
    } else {
	okB->setEnabled( FALSE );
    }	

    if ( d->currentFileName.length() && old != d->currentFileName )
	emit fileHighlighted( d->currentFileName );

    return !d->currentFileName.isNull();
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
    if ( d->extraLabel ) {
	t = d->extraLabel->sizeHint();
	RM;
    }
    d->pathL->setFixedSize( r );
    d->fileL->setFixedSize( r );
    d->typeL->setFixedSize( r );
    if ( d->extraLabel )
	d->extraLabel->setFixedSize( r );

    // single-line input areas
    r = d->paths->sizeHint();
    t = nameEdit->sizeHint();
    RM;
    t = d->types->sizeHint();
    RM;
    r.setWidth( t.width() * 2 / 3 );
    if ( d->extraWidget ) {
	t = d->extraWidget->sizeHint();
	RM;
    }
    t.setWidth( QCOORD_MAX );
    t.setHeight( r.height() );
    d->paths->setMinimumSize( r );
    d->paths->setMaximumSize( t );
    nameEdit->setMinimumSize( r );
    nameEdit->setMaximumSize( t );
    d->types->setMinimumSize( r );
    d->types->setMaximumSize( t );
    if ( d->extraWidget ) {
	d->extraWidget->setMinimumSize( r );
	d->extraWidget->setMaximumSize( t );
    }

    // buttons on top row
    r = QSize( 0, d->paths->minimumSize().height() );
    t = QSize( 21, 20 );
    RM;
    if ( r.height()+1 > r.width() )
	r.setWidth( r.height()+1 );
    d->cdToParent->setFixedSize( r );
    d->mcView->setFixedSize( r );
    d->detailView->setFixedSize( r );
    // ...

    // open/save, cancel
    r = QSize( 75, 20 );
    t = okB->sizeHint();
    RM;
    t = cancelB->sizeHint();
    RM;
    if ( d->extraButton ) {
	t = d->extraButton->sizeHint();
	RM;
    }
    okB->setFixedSize( r );
    cancelB->setFixedSize( r );
    if ( d->extraButton )
	d->extraButton->setFixedSize( r );

    d->topLevelLayout->activate();

#undef RM
}


/*!  Updates the dialog when the cursor moves in the listview. */

void QFileDialog::updateFileNameEdit( QListViewItem * newItem )
{
    if ( !newItem )
	return;

    if ( mode() == ExistingFiles ) {
	bool ok = files->isSelected( newItem );
	QListViewItem * i = files->firstChild();
	while( i && !ok ) {
	    ok = i->isSelected();
	    i = i->nextSibling();
	}
	okB->setText( tr( "OK" ) );
	okB->setEnabled( ok );
    } else if ( files->isSelected( newItem ) ) {
	QFileDialogPrivate::File * i = (QFileDialogPrivate::File *)newItem;
	trySetSelection( i->info, TRUE );
    }
}


/*! \overload */

void QFileDialog::updateFileNameEdit( QListBoxItem * newItem )
{
    if ( !newItem )
	return;
    QFileDialogPrivate::MCItem * i = (QFileDialogPrivate::MCItem *)newItem;
    i->i->setSelected( i->selected() );
    updateFileNameEdit( i->i );
}


/*!  Updates the dialog when the file name edit changes. */

void QFileDialog::fileNameEditDone()
{
    QFileInfo f ( cwd, nameEdit->text() );
    trySetSelection( f, FALSE );
}



/*!  This private slot reacts to double-clicks in the list view. */

void QFileDialog::selectDirectoryOrFile( QListViewItem * newItem )
{
    if ( !newItem )
	return;

    QFileDialogPrivate::File * i = (QFileDialogPrivate::File *)newItem;

    if ( i->info.isDir() ) {
	if ( mode() == ExistingFiles ) {
	    QListViewItem * i = files->firstChild();
	    while( i && !i->isSelected() )
		i = i->nextSibling();
	    if ( i ) {
		accept();
		return;
	    }
	}
	setDir( i->info.absFilePath() );
	if ( mode() == Directory ) {
	    QFileInfo f ( cwd, "." );
	    trySetSelection( f, TRUE );
	}
    } else if ( newItem->isSelectable() && trySetSelection( i->info, TRUE ) ) {
	if ( mode() != Directory ) {
	    emit fileSelected( d->currentFileName );
	    accept();
	}
    }
}


void QFileDialog::selectDirectoryOrFile( QListBoxItem * newItem )
{
    if ( !newItem )
	return;
    QFileDialogPrivate::MCItem * i = (QFileDialogPrivate::MCItem *)newItem;
    i->i->setSelected( i->selected() );
    selectDirectoryOrFile( i->i );
}


void QFileDialog::popupContextMenu( QListViewItem *, const QPoint & p,
				    int c )
{
    QPopupMenu m( 0, "file dialog context menu" );

    int asc = m.insertItem( tr("&Ascending") );
    int desc = m.insertItem( tr("&Descending") );

    m.move( p );
    int res = m.exec();
    if ( res == asc )
	files->setSorting( c, TRUE );
    else if ( res == desc )
	files->setSorting( c, FALSE );
}


void QFileDialog::fileSelected( int  )
{
    // unused
}

void QFileDialog::fileHighlighted( int )
{
    // unused
}

void QFileDialog::dirSelected( int )
{
    // unused
}

void QFileDialog::pathSelected( int )
{
    // unused
}


void QFileDialog::cdUpClicked()
{
    if ( cwd.cdUp() ) {
	cwd.convertToAbs();
	rereadDir();
    }
}


/*!  Ask the user for the name of an existing directory, starting at
  \a dir.  Returns the name of the directory the user selected.

  If \a dir is null, getExistingDirectory() starts wherever the
  previous file dialog left off.
*/

QString QFileDialog::getExistingDirectory( const QString & dir,
					   QWidget *parent,
					   const char* name )
{
    makeVariables();
    QFileDialog *dialog	= new QFileDialog( parent, name, TRUE );
    dialog->setCaption( dialog->tr("Find Directory") );

    dialog->setMode( Directory );

    dialog->d->types->clear();
    dialog->d->types->insertItem( dialog->tr("Directories") );
    dialog->d->types->setEnabled( FALSE );

    if ( !workingDirectory->isEmpty() ) {
	QFileInfo f( *workingDirectory );
	if ( f.isDir() )
	    dialog->setDir( *workingDirectory );
    }	
    if ( !dir.isEmpty() ) {
	QFileInfo f( dir );
	if ( f.isDir() ) {
	    *workingDirectory = dir;
	    dialog->setDir( dir );
	}
    }

    QString result;
    if ( dialog->exec() == QDialog::Accepted ) {
	result = dialog->selectedFile();
	QFileInfo f( result );
	if ( f.isDir() ) {
	    *workingDirectory = result;
	} else {
	    result = 0;
	}
    }
    delete dialog;
    return result;
}


/*!  Sets this file dialog to \a newMode, which can be one of \c
  Directory (directories are accepted), \c ExistingFile (existing
  files are accepted) or \c AnyFile (any valid file name is accepted).

  \sa mode()
*/

void QFileDialog::setMode( Mode newMode )
{
    if ( d->mode != newMode ) {
	//cwd.setFilter( QDir::All );
	d->mode = newMode;
	QString sel = d->currentFileName;
	if ( newMode == Directory ) {
	    files->setMultiSelection( FALSE );
	    if ( sel.isNull() )
		sel = ".";
	} else if ( newMode == ExistingFiles ) {
	    files->setMultiSelection( TRUE );
	} else {
	    files->setMultiSelection( FALSE );
	}
	rereadDir();
	QFileInfo f ( cwd, sel );
	trySetSelection( f, TRUE );
    }
}


/*!  Returns the file mode of this dialog.

  \sa setMode()
*/

QFileDialog::Mode QFileDialog::mode() const
{
    return d->mode;
}


/*!  Adds 1-3 widgets to the bottom of the file dialog.  \a l is the
  (optional) label, which is put beneath the "file name" and "file
  type" labels, \a w is a (optional) widget, which is put beneath the
  file type combo box, and \a b is the (you guessed it - optional)
  button, which is put beneath the cancel button.

  If you don't want to add something in one of the columns, pass 0.

  It is not currently possible to add more than one row.
*/

void QFileDialog::addWidgets( QLabel * l, QWidget * w, QPushButton * b )
{
    if ( !l && !w && !b )
	return;
    if ( d->extraLabel || d->extraWidget || d->extraButton )
	return;

    d->extraWidgetsLayout = new QHBoxLayout();
    d->topLevelLayout->addLayout( d->extraWidgetsLayout );

    d->extraLabel = l;
    if ( l )
	d->extraWidgetsLayout->addWidget( l );
    d->extraWidget = w;
    if ( w )
	d->extraWidgetsLayout->addWidget( w );
    d->extraButton = b;
    if ( b )
	d->extraWidgetsLayout->addWidget( b );

    d->topLevelLayout->activate();
    updateGeometry();
}


/*! \reimp */

void QFileDialog::keyPressEvent( QKeyEvent * ke )
{
    if ( ke && ( ke->key() == Key_Enter ||
		 ke->key() == Key_Return ) ) {
	ke->ignore();
	if ( d->paths->hasFocus() ) {
	    ke->accept();
	    if ( cwd.absPath() == d->paths->currentText() )
		nameEdit->setFocus();
	} else if ( d->types->hasFocus() ) {
	    ke->accept();
	    // ### is there a suitable condition for this?  only valid
	    // wildcards?
	    nameEdit->setFocus();
	} else if ( nameEdit->hasFocus() ) {
	    if ( d->currentFileName.isNull() ) {
		// maybe change directory
		QFileInfo i( cwd, nameEdit->text() );
		if ( i.isDir() ) {
		    nameEdit->setText( "" );
		    setDir( i.filePath() );
		}
		ke->accept();
	    } else if ( mode() == ExistingFiles ) {
		QFileInfo i( cwd, nameEdit->text() );
		if ( i.isFile() ) {
		    QListViewItem * i = files->firstChild();
		    while ( i && qstrcmp( nameEdit->text(), i->text( 0 ) ) )
			i = i->nextSibling();
		    if ( i )
			 files->setSelected( i, TRUE );
		    else
			ke->accept(); // strangely, means to ignore that event
		}
	    }
	} else if ( files->hasFocus() || d->moreFiles->hasFocus() ) {
	    ke->accept();
	}
    } else if ( ke->key() == Key_Escape ) {
	ke->ignore();
    }

    if ( !ke->isAccepted() ) {
	QDialog::keyPressEvent( ke );
    }
}


/*! \class QFileIconProvider qfiledialog.h

  \brief The QFileIconProvider class provides icons for QFileDialog to
  use.

  By default, QFileIconProvider is not used, but any application or
  library can subclass it, reimplement pixmap() to return a suitable
  icon, and make all QFileDialog objects use it by calling the static
  function QFileDialog::setIconProvider().

  It's advisable to make all the icons QFileIconProvider returns be of
  the same size, or at least the same width.  This makes the list view
  look much better.

  \sa QFileDialog
*/


/*!  Constructs an empty file icon provider. */

QFileIconProvider::QFileIconProvider( QObject * parent, const char* name )
    : QObject( parent, name )
{
    // nothing necessary
}


/*!  Returns a pointer to a pixmap suitable for display when the file
  dialog next to the name of \a file.

  If pixmap() returns 0, QFileDialog draws nothing.

  The default implementation returns 0 in Qt 1.40.  In future versions
  of Qt it may be extended.
*/

const QPixmap * QFileIconProvider::pixmap( const QFileInfo & )
{
    return 0;
}


/*!  Sets all file dialogs to use \a provider to select icons to draw
  for each file.  By default there is no icon provider, and
  QFileDialog simply draws a "folder" icon next to each directory and
  nothing next to the files.

  \sa QFileIconProvider iconProvider()
*/

void QFileDialog::setIconProvider( QFileIconProvider * provider )
{
    fileIconProvider = provider;
}


/*!  Returns the icon provider currently in use.  By default there is
  no icon provider and this function returns 0.

  \sa setIconProvider() QFileIconProvider
*/

QFileIconProvider * QFileDialog::iconProvider()
{
    return fileIconProvider;
}


/*! \reimp */

bool QFileDialog::eventFilter( QObject * o, QEvent * e )
{
    if ( !o || !e )
	return TRUE;
    if ( mode() == ExistingFiles &&
	 e->type() == QEvent::MouseButtonDblClick &&
	 ( o == files || o == d->moreFiles || o == files->viewport() ) ) {
	QListViewItem * i = files->firstChild();
	while( i && !i->isSelected() )
	    i = i->nextSibling();
	if ( i )
	    return TRUE;
    } else if ( e->type() == QEvent::KeyPress &&
		((QKeyEvent *)e)->key() == Key_Backspace &&
		( o == files ||
		  o == d->moreFiles ||
		  o == files->viewport() ) ) {
	cdUpClicked();
	((QKeyEvent *)e)->accept();
	return TRUE;
    } else if ( o == files && e->type() == QEvent::FocusOut &&
	 files->currentItem() && mode() != ExistingFiles ) {
	files->setSelected( files->currentItem(), FALSE );
    } else if ( o == files && e->type() == QEvent::KeyPress ) {
	QTimer::singleShot( 0, this, SLOT(fixupNameEdit()) );
    } else if ( o == nameEdit && e->type() == QEvent::KeyPress ) {
	// ### hack.  after 1.40, we need to merge the completion code
	// ### here, in QListView and QComboBox.
	if ( isprint(((QKeyEvent *)e)->ascii()) ) {
	    QString nt( nameEdit->text() );;
	    nt.truncate( nameEdit->cursorPosition() );
	    nt += (char)(((QKeyEvent *)e)->ascii());
	    QListViewItem * i = files->firstChild();
	    while( i && qstrncmp( i->text( 0 ), nt, nt.length() ) )
		i = i->nextSibling();
	    if ( i ) {
		nt = i->text( 0 );
		int cp = nameEdit->cursorPosition()+1;
		nameEdit->validateAndSet( nt, cp, cp, nt.length() );
		return TRUE;
	    }
	}
    } else if ( o == nameEdit && e->type() == QEvent::FocusIn ) {
	fileNameEditDone();
    }
    return FALSE;
}


/*!  Sets this file dialog to offer \a types in the File Type combo
  box.  \a types must be a null-terminated list of strings; each
  string must be in the format described in the documentation for
  setFilter().

  \sa setFilter()
*/

void QFileDialog::setFilters( const char ** types )
{
    if ( !types || !*types )
	return;

    d->types->clear();
    while( types && *types ) {
	d->types->insertItem( *types );
	types++;
    }
    d->types->setCurrentItem( 0 );
    setFilter( d->types->text( 0 ) );
}


/*! \overload void QFileDialog::setFilters( const QStrList & )
*/

void QFileDialog::setFilters( const QStrList & types )
{
    if ( types.count() < 1 )
	return;

    d->types->clear();
    QStrListIterator it( types );
    it.toFirst();
    QString t;
    while( (t=it.current()) != 0 ) {
	++it;
	d->types->insertItem( t );
    }
    d->types->setCurrentItem( 0 );
    setFilter( d->types->text( 0 ) );
}


/*!
  Since modeButtons is a top-level widget, it may be destroyed by the
  kernel at application exit time. Notice if this happens to
  avoid double deletion.
*/

void QFileDialog::modeButtonsDestroyed()
{
    if ( d )
	d->modeButtons = 0;
}


/*!  Lets the user select N files from a single directory, and returns
  a list of the selected files.  The list may be empty, and the file
  names are fully qualified (i.e. "/usr/games/quake" or
  "c:\\quake\\quake").

  \a filter is the default glob pattern (which the user can change).
  The default is all files.  \a dir is the starting directory.  If \a
  dir is not supplied, QFileDialog picks something presumably useful
  (such as the directory where the user selected something last, or
  the current working directory).

  \a parent is a widget over which the dialog should be positioned and
  \a name is the object name of the temporary QFileDialog object.

  Note that the returned list has auto-delete turned off.  It is the
  application developer's responsibility to delete the strings in the
  list, for example using code such as:

  \code
    QStrList s( QFileDialog::getOpenFileNames() );
    // do something with the files in s.
    s.setAutoDelete();
    s.clear(); // or just go out of scope.
  \endcode
*/

QStrList QFileDialog::getOpenFileNames( const QString & filter,
					const QString& dir,
					QWidget *parent,
					const char* name )
{
    makeVariables();

    if ( workingDirectory->isNull() )
	*workingDirectory = QDir::currentDirPath();

    QFileInfo tmp( QDir::root(), dir );
    if ( !tmp.isDir() ) {
	tmp.setFile( QDir::root(), *workingDirectory );
	while( !tmp.isDir() )
	    tmp.setFile( tmp.dirPath( TRUE ) );
    }

    *workingDirectory = tmp.absFilePath();
    QFileDialog *dlg = new QFileDialog( *workingDirectory, filter,
					parent, name, TRUE );
    CHECK_PTR( dlg );
    dlg->setCaption( dlg->tr("Open") );
    dlg->setMode( QFileDialog::ExistingFiles );
    QString result;
    QStrList s;
    s.setAutoDelete( FALSE );
    if ( dlg->exec() == QDialog::Accepted ) {
	QListViewItem * i = dlg->files->firstChild();
	while( i ) {
	    if ( i->isSelected() )
		s.append( ((QFileDialogPrivate::File*)i)->info.absFilePath() );
	    i = i->nextSibling();
	}
	*workingDirectory = dlg->dirPath();
    }
    delete dlg;
    return s;
}



/*!  Updates the line edit to match the speed-key usage in QListView. */

void QFileDialog::fixupNameEdit()
{
    if ( files->currentItem() )
	nameEdit->setText( files->currentItem()->text( 0 ) );
}
