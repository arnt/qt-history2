/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "book.h"
#include "editbook.h"

class EditBookForm : public EditBookFormBase
{
public:
    EditBookForm::EditBookForm( 
	    int action = 0,
	    QWidget *parent = 0, const char *name = 0, bool modal = FALSE, WFlags f = 0 ); 
    enum { UPDATE = 0, INSERT, DELETE };
private:
};


