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
#include <qdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qsqldatabase.h>
#include <qsqlcursor.h>
#include <qsqlform.h>

bool create_connections();


class FormDialog : public QDialog
{
    Q_OBJECT
    public:
	FormDialog();
	~FormDialog();
    public slots:
	void slotSave();
    private:
	QSqlCursor *staffCursor;
	QSqlForm *sqlForm;
	QSqlIndex idIndex;
};


