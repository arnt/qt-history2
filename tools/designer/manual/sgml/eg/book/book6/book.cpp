/****************************************************************************
** Form implementation generated from reading ui file '/home/db/src/qt/main/tools/designer/manual/sgml/eg/book/book6/book.ui'
**
** Created: Fri Feb 23 12:24:20 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "/home/db/src/qt/main/tools/designer/manual/sgml/eg/book/book6/book.h"

#include <qvariant.h>   // first for gcc 2.7.2
#include <qdatatable.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include <qsqlcursor.h>
#include <qsqldatabase.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a BookForm which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
BookForm::BookForm( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "BookForm" );
    resize( 498, 508 ); 
    setCaption( tr( "Book" ) );
    BookFormLayout = new QVBoxLayout( this ); 
    BookFormLayout->setSpacing( 6 );
    BookFormLayout->setMargin( 11 );

    Splitter1 = new QSplitter( this, "Splitter1" );
    Splitter1->setFrameShape( QSplitter::MShape );
    Splitter1->setFrameShadow( QSplitter::MShadow );
    Splitter1->setOrientation( QSplitter::Vertical );

    AuthorDataTable = new QDataTable( Splitter1, "AuthorDataTable" );
    AuthorDataTable->addColumn( "surname", "Surname" );
    AuthorDataTable->addColumn( "forename", "Forename" );
    AuthorDataTable->setConfirmDelete( TRUE );
    QStringList AuthorDataTableSort;
    AuthorDataTableSort << "surname ASC" << "forename ASC";
    AuthorDataTable->setSort( AuthorDataTableSort );

    BookDataTable = new QDataTable( Splitter1, "BookDataTable" );
    BookDataTable->addColumn( "title", "Title" );
    BookDataTable->addColumn( "price", "Price" );
    BookDataTable->addColumn( "notes", "Notes" );
    BookDataTable->setReadOnly( TRUE );
    QStringList BookDataTableSort;
    BookDataTableSort << "title ASC";
    BookDataTable->setSort( BookDataTableSort );
    BookFormLayout->addWidget( Splitter1 );

    Layout5 = new QHBoxLayout; 
    Layout5->setSpacing( 6 );
    Layout5->setMargin( 0 );

    EditPushButton = new QPushButton( this, "EditPushButton" );
    EditPushButton->setText( tr( "&Edit Books" ) );
    Layout5->addWidget( EditPushButton );

    QuitPushButton = new QPushButton( this, "QuitPushButton" );
    QuitPushButton->setText( tr( "&Quit" ) );
    Layout5->addWidget( QuitPushButton );
    BookFormLayout->addLayout( Layout5 );

    // database support





    // signals and slots connections
    connect( QuitPushButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( EditPushButton, SIGNAL( clicked() ), this, SLOT( editClicked() ) );
    connect( AuthorDataTable, SIGNAL( primeInsert(QSqlRecord*) ), this, SLOT( primeInsertAuthor(QSqlRecord*) ) );
    connect( AuthorDataTable, SIGNAL( currentChanged(QSqlRecord*) ), this, SLOT( newCurrentAuthor(QSqlRecord*) ) );
    init();
}

/*  
 *  Destroys the object and frees any allocated resources
 */
BookForm::~BookForm()
{
    destroy();
    // no need to delete child widgets, Qt does it all for us
}

/*  
 *  Widget polish.  Reimplemented to handle
 *  default data table initialization
 */
void BookForm::polish()
{
    if ( AuthorDataTable ) {
        QSqlCursor* cursor = AuthorDataTable->sqlCursor();
        if ( !cursor ) {
            cursor = new QSqlCursor( "author" );
            AuthorDataTable->setCursor( cursor, FALSE, TRUE );
        }
        if ( !cursor->isActive() )
            AuthorDataTable->refresh( QDataTable::RefreshAll );
    }
    if ( BookDataTable ) {
        QSqlCursor* cursor = BookDataTable->sqlCursor();
        if ( !cursor ) {
            cursor = new QSqlCursor( "book" );
            BookDataTable->setCursor( cursor, FALSE, TRUE );
        }
        if ( !cursor->isActive() )
            BookDataTable->refresh( QDataTable::RefreshAll );
    }
    QDialog::polish();
}

void BookForm::editClicked()
{
    EditBookForm *dialog = new EditBookForm( this, "Edit Book Form", TRUE );
    QSqlCursor cur( "book" );
    dialog->BookDataBrowser->setCursor( &cur );
    dialog->BookDataBrowser->setFilter( BookDataTable->filter() );
    dialog->BookDataBrowser->setSort(QSqlIndex::fromStringList( 
    	BookDataTable->sort(), &cur ) );
    dialog->BookDataBrowser->refresh();
    int i = BookDataTable->currentRow();
    if ( i == -1 ) i = 0; // Always use the first row
    dialog->BookDataBrowser->seek( i );
    dialog->exec();
    delete dialog;
    BookDataTable->refresh();
}

void BookForm::newCurrentAuthor( QSqlRecord * author )
{
    BookDataTable->setFilter( "authorid=" + author->value( "id" ).toString() );  
    BookDataTable->refresh();
}

void BookForm::primeInsertAuthor( QSqlRecord * buffer )
{
    QSqlQuery q; 
    q.exec( "update sequence set sequence = sequence + 1 where tablename='author';" ); 
    q.exec( "select sequence from sequence where tablename='author';" ); 
    if ( q.next() ) { 
	buffer->setValue( "id", q.value( 0 ) ); 
    } 
}

void BookForm::init()
{
}

void BookForm::destroy()
{
}

