/****************************************************************************
** Form implementation generated from reading ui file '/home/mark/p4/qt/tools/designer/manual/sgml/eg/book/book2/book.ui'
**
** Created: Fri Jan 19 15:50:50 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "/home/mark/p4/qt/tools/designer/manual/sgml/eg/book/book2/book.h"

#include <qvariant.h>   // first for gcc 2.7.2
#include <qdatatable.h>
#include <qpushbutton.h>
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
    resize( 588, 474 ); 
    setCaption( tr( "Book" ) );
    BookFormLayout = new QVBoxLayout( this ); 
    BookFormLayout->setSpacing( 6 );
    BookFormLayout->setMargin( 11 );

    AuthorcDataTable = new QDataTable( this, "AuthorcDataTable" );
    AuthorcDataTable->addColumn( "surname", "Surname" );
    AuthorcDataTable->addColumn( "forename", "Forename" );
    AuthorcDataTable->setConfirmDelete( TRUE );
    QStringList AuthorcDataTableSort;
    AuthorcDataTableSort << "surname ASC" << "forename ASC";
    AuthorcDataTable->setSort( AuthorcDataTableSort );
    BookFormLayout->addWidget( AuthorcDataTable );

    // database support





    // signals and slots connections
    connect( AuthorcDataTable, SIGNAL( primeInsert(QSqlRecord*) ), this, SLOT( primeInsertAuthor(QSqlRecord*) ) );
    init();
}

/*  
 *  Destroys the object and frees any allocated resources
 */
BookForm::~BookForm()
{
    // no need to delete child widgets, Qt does it all for us
}

/*  
 *  Widget polish.  Reimplemented to handle
 *  default data table initialization
 */
void BookForm::polish()
{
    if ( AuthorcDataTable ) {
        QSqlCursor* cursor = AuthorcDataTable->sqlCursor();
        if ( !cursor ) {
            cursor = new QSqlCursor( "author" );
            AuthorcDataTable->setCursor( cursor, FALSE, TRUE );
        }
        if ( !cursor->isActive() )
            AuthorcDataTable->refresh( QDataTable::RefreshAll );
    }
    QDialog::polish();
}

void BookForm::primeInsertAuthor( QSqlRecord *buffer )
{
    QSqlQuery query( "SELECT nextval('author_seq');" );
    if ( query.next() )
	buffer->setValue( "id", query.value( 0 ) );
}

void BookForm::init()
{
}

