/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qsqldatabase.h>
#include <qsqltable.h>
#include "book.h"

class QSqlRecord;


class BookForm : public BookFormBase
{
    Q_OBJECT
public slots:
    void slotNewAuthor( const QSqlRecord *authorRecord );
};

