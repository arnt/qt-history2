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
#include <qsqlcursor.h>

class QSqlRecord;

bool createConnections();


class InvoiceItemCursor : public QSqlCursor
{
    public:
	InvoiceItemCursor();
    protected:
	QVariant calculateField( const QString & name );
};


