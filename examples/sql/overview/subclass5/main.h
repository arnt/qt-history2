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
#include <qdatetime.h>
#include <qsqldatabase.h>
#include <qsqlcursor.h>
#include "../login.h"

class QSqlRecord;

bool createConnections();


class InvoiceItemCursor : public QSqlCursor
{
    public:
	InvoiceItemCursor();
	QSqlRecord *primeInsert();
    protected:
	QVariant calculateField( const QString & name );
};
