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
#include <qsqlpropertymap.h>

bool create_connections();


class CustomEdit : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY( QString upperLine READ upperLine WRITE setUpperLine )
    public:
	CustomEdit( QWidget *parent=0, const char *name=0 );
	QString upperLine() const;
	void setUpperLine( const QString &line );
    public slots:
	void slotChanged( const QString &line );
    private:
	QString upper_line;
};


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
	QSqlPropertyMap *propMap;
	QSqlIndex idIndex;
};


