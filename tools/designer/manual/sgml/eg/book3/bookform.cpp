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

AuthorCursor::AuthorCursor() :
    QSqlCursor( "author" )
{
    setDisplayLabel( "forename", "Forename" );
    setDisplayLabel( "surname", "Surname" );
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


void BookForm::slotNewAuthor( const QSqlRecord *authorRecord )
{
    BookSqlTable->setFilter( 
	"book_view.authorid=" + authorRecord->value( "id" ).toString() );
    BookSqlTable->refresh();
}



