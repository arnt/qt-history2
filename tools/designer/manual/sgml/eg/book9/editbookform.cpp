/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "editbookform.h"


EditBookForm::EditBookForm( 
    const QString &bookID, int action,
    QWidget *parent ) :
    EditBookFormBase( parent, "edit bookform", TRUE, WDestructiveClose ) 
{
    if ( action != INSERT )
	setFilter( "id=" + bookID );
    
    PushButtonUpdate->setEnabled( FALSE );
    PushButtonInsert->setEnabled( FALSE );
    PushButtonDelete->setEnabled( FALSE );

    QLineEditTitle->setEnabled( TRUE );	
    QLineEditPrice->setEnabled( TRUE );	

    switch ( action ) {
	case UPDATE: 
	    setCaption( "Update Book" );
	    PushButtonUpdate->setEnabled( TRUE );
	    break;
	case INSERT: 
	    setCaption( "Insert Book" );
	    PushButtonInsert->setEnabled( TRUE );
	    break;
	case DELETE: 
	    QLineEditTitle->setEnabled( FALSE );	
	    QLineEditPrice->setEnabled( FALSE );	
	    setCaption( "Delete Book" );
	    PushButtonDelete->setEnabled( TRUE );
	    break;
    }
}



