/****************************************************************************
**
** Implementation of QDataView class
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

#include "qdataview.h"

#ifndef QT_NO_SQL

#include "qsqlmanager_p.h"

class QDataView::QDataViewPrivate
{
public:
    QDataViewPrivate() {}
    QSqlFormManager frm;
};


/*!

  \class QDataView qdataview.h
  \brief This class provides SQL forms that support navigation but are
  read-only.

  \module sql

    This class provides SQL forms that users can navigate to view data.
    Because QDataViews don't support editing they use less resources
    than QDataBrowsers making them a good choice for read-only forms.

*/

/*! Creates a data view.

*/

QDataView::QDataView( QWidget *parent, const char *name, WFlags fl )
    : QWidget( parent, name, fl )
{
    d = new QDataViewPrivate();
}

/*! Destroys the object and frees any allocated resources.

*/

QDataView::~QDataView()
{
    delete d;
}

/*!  Clears the default form's values.  If there is no default form,
  nothing happens. All the values are set to their 'zero state', e.g. 0
  for numeric fields, "" for string fields. 

*/

void QDataView::clearValues()
{
    d->frm.clearValues();
}

/*! Sets the form used by the data view to \a form.  If a record has
  already been assigned to the data view, the form will display that
  record's data.

  \sa form()

*/

void QDataView::setForm( QSqlForm* form )
{
    d->frm.setForm( form );
}


/*! Returns the default form used by the data view, or 0 if there is
  none.

  \sa setForm()

*/

QSqlForm* QDataView::form()
{
    return d->frm.form();
}


/*! Sets the record used by the data view to \a record.  If a form has
  already been assigned to the data view, the form will display this \a
  record's data.

  \sa record()

*/

void QDataView::setRecord( QSqlRecord* record )
{
    d->frm.setRecord( record );
}


/*! Returns the default record used by the data view, or 0 if there is
  none.

  \sa setRecord()
*/

QSqlRecord* QDataView::record()
{
    return d->frm.record();
}


/*! Causes the default form to read its fields from the record buffer.  If
   there is no default form, or no record, nothing happens.

  \sa setForm()

*/

void QDataView::readFields()
{
    d->frm.readFields();
}

/*! Causes the default form to write its fields to the record buffer.
   If there is no default form, or no record, nothing happens.

  \sa setForm()

*/

void QDataView::writeFields()
{
    d->frm.writeFields();
}

/*! Causes the default form to display the contents of \a record.  The
  \a record also becomes the default record for all subsequent calls
  to readFields() and writefields() .  If there is no default form,
  nothing happens.  This slot is equivelant to calling:

  \code
  myView.setRecord( record );
  myView.readFields();
  \endcode

  \sa setRecord() readFields()

*/

void QDataView::refresh( QSqlRecord* buf )
{
    if ( buf && buf != record() )
	setRecord( buf );
    readFields();
}

#endif
