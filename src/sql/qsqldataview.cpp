/****************************************************************************
**
** Implementation of QSqlDataView class
**
** Created : 2000-11-03
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qsqldataview.h"

#ifndef QT_NO_SQL

#include "qsqlform.h"

class QSqlDataView::QSqlDataViewPrivate
{
public:
    QSqlDataViewPrivate() : frm(0), rcd(0) {}
    QSqlForm* frm;
    QSqlRecord* rcd;
};


/*!

  \class QSqlDataView qsqldataview.h
  \brief SQL form manipulation

  \module sql

  This class is used to ...//###

*/

/*! Creates a data view.

*/

QSqlDataView::QSqlDataView( QWidget *parent, const char *name, WFlags fl )
    : QWidget( parent, name, fl )
{
    d = new QSqlDataViewPrivate();
}

/*! Destroys the object and frees any allocated resources.

*/

QSqlDataView::~QSqlDataView()
{
    delete d;
}

/*!  Clears the default form values.  If there is no default form,
  nothing happens,

*/

void QSqlDataView::clearFormValues()
{
    if ( form() )
	form()->clearValues();
}

/*! Sets the form used by the data view to \a form.  If a record has
  already been assigned to the data view, that record is also used by
  the \a form to display data.

  \sa form()

*/

void QSqlDataView::setForm( QSqlForm* form )
{
    d->frm = form;
    if ( d->rcd )
	d->frm->setRecord( d->rcd );
}


/*! Returns the default form used by the data view, or 0 if there is
  none.

  \sa setForm()

*/

QSqlForm* QSqlDataView::form()
{
    return d->frm;
}


/*! Sets the record used by the data view to \a record.  If a form has
  already been assigned to the data view, \a record is also used by
  the default form to display data.

  \sa record()

*/

void QSqlDataView::setRecord( QSqlRecord* record )
{
    d->rcd = record;
    if ( d->frm )
	d->frm->setRecord( d->rcd );
}


/*! Returns the default record used by the data view, or 0 if there is
  none.

  \sa setRecord()
*/

QSqlRecord* QSqlDataView::record()
{
    return d->rcd;
}


/*! Causes the default form to read its fields .  If there is no
  default form, nothing happens.

  \sa setForm()

*/

void QSqlDataView::readFormFields()
{
    if ( d->frm )
	d->frm->readFields();
}

/*! Causes the default form to write its fields .  If there is no
  default form, nothing happens.

  \sa setForm()

*/

void QSqlDataView::writeFormFields()
{
    if ( d->frm )
	d->frm->writeFields();
}

#endif
