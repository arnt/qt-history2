/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
#include "../connection.h"

class CustomEdit : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY( QString upperLine READ upperLine WRITE setUpperLine )
    public:
	CustomEdit( QWidget *parent=0, const char *name=0 );
	QString upperLine() const;
	void setUpperLine( const QString &line );
    public slots:
	void changed( const QString &line );
    private:
	QString upperLineText;
};


class FormDialog : public QDialog
{
    Q_OBJECT
    public:
	FormDialog();
	~FormDialog();
    public slots:
	void save();
    private:
	QSqlCursor *staffCursor;
	QSqlForm *sqlForm;
	QSqlPropertyMap *propMap;
	QSqlIndex idIndex;
};


