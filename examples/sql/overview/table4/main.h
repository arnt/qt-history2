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
#include <qcombobox.h>
#include <qmap.h>
#include <qpainter.h>
#include <qsqldatabase.h>
#include <qsqlcursor.h>
#include <qsqleditorfactory.h>
#include <qsqlpropertymap.h>
#include <qdatatable.h>

bool createConnections();


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


class CustomTable : public QDataTable
{
    Q_OBJECT
public:
    CustomTable::CustomTable( QSqlCursor * cursor, bool autoPopulate = FALSE, QWidget * parent = 0, const char * name = 0 ) : QDataTable( cursor, autoPopulate, parent, name ) {}
    void CustomTable::paintField(
	    QPainter * p, const QSqlField* field, const QRect & cr, bool );

};


class CustomSqlEditorFactory : public QSqlEditorFactory
{
    Q_OBJECT
public:
    QWidget *createEditor( QWidget *parent, const QSqlField *field );
};
