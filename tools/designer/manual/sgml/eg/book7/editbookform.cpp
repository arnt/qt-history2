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
    int action, 
    QWidget *parent, const char *name, bool modal, WFlags f ) :
    EditBookFormBase( parent, name, modal, f ) 
{
    switch ( action ) {
	case UPDATE: 
	    setCaption( "Update Book" );
	    break;
	case INSERT: 
	    setCaption( "Insert Book" );
	    break;
	case DELETE: 
	    setCaption( "Delete Book" );
	    break;
    }
}



