/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "bookform.h"
#include "editbookform.h"

AuthorCursor::AuthorCursor() :
    QSqlCursor( "author" )
{
    // NOOP
}


QSqlRecord *AuthorCursor::primeInsert()
{
    QSqlRecord *record = editBuffer();
    QSqlQuery q( "SELECT nextval( 'author_seq' );" );
    if ( q.next() ) {
	record->setValue( "id", q.value( 0 ) );
    }

    return record;
}


BookForm::BookForm( QWidget * parent, const char * name, bool modal, WFlags f ) :
    BookFormBase( parent, name, modal, f )
{
    AuthorSqlTable->setCursor( &authorCursor );
    AuthorSqlTable->refresh();
    QSqlRecord firstAuthor = AuthorSqlTable->currentFieldSelection();
    BookSqlTable->setFilter( 
	"book_view.authorid=" + firstAuthor.value( "id" ).toString() );
    BookSqlTable->refresh();
}


void BookForm::newAuthor( const QSqlRecord *authorRecord )
{
    BookSqlTable->setFilter( 
	"book_view.authorid=" + authorRecord->value( "id" ).toString() );
    BookSqlTable->refresh();
}



void BookForm::insertBook()
{
    QSqlRecord book = BookSqlTable->currentFieldSelection();
    EditBookForm *dialog = new EditBookForm( 
				book.value( "id" ).toString(),
				EditBookForm::INSERT, this );
    dialog->exec();
    BookSqlTable->refresh();
}


void BookForm::updateBook()
{
    QSqlRecord book = BookSqlTable->currentFieldSelection();
    EditBookForm *dialog = new EditBookForm( 
				book.value( "id" ).toString(),
				EditBookForm::UPDATE, this );
    dialog->exec();
    BookSqlTable->refresh();
}


void BookForm::deleteBook()
{
    QSqlRecord book = BookSqlTable->currentFieldSelection();
    EditBookForm *dialog = new EditBookForm( 
				book.value( "id" ).toString(),
				EditBookForm::DELETE, this );
    dialog->exec();
    BookSqlTable->refresh();
}



