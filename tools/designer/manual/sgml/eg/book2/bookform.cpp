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


void BookForm::slotNewAuthor( const QSqlRecord *authorRecord )
{
    BookSqlTable->setFilter( 
	"book_view.authorid=" + authorRecord->value( "id" ).toString() );
    BookSqlTable->refresh();
}



