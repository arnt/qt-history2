/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qpushbutton.h>
#include <qsqlcursor.h>
#include <qlineedit.h>
#include "book.h"
#include "editbook.h"

class QSqlRecord;


class EditBookForm : public EditBookFormBase
{
public:
    EditBookForm::EditBookForm( 
	    const QString &bookID, int action = 0, QWidget *parent = 0 ); 
    enum { UPDATE = 0, INSERT, DELETE };
};


