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
#include <qcombobox.h>
#include <qmap.h>
#include <qsqldatabase.h>
#include <qsqlcursor.h>
#include <qsqleditorfactory.h>
#include <qsqlpropertymap.h>
#include "../connection.h"

class StatusPicker : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY( int statusid READ statusId WRITE setStatusId )
    public:
	StatusPicker( QWidget *parent=0, const char *name=0 );
	int statusId() const;
	void setStatusId( int id );
    private:
	QMap< int, int > index2id;
};


class CustomSqlEditorFactory : public QSqlEditorFactory
{
    Q_OBJECT
    public:
	QWidget *createEditor( QWidget *parent, const QSqlField *field );
};



