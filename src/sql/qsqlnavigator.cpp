/****************************************************************************
**
** Implementation of QSqlNavigator class
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

#include "qsqlnavigator.h"

#ifndef QT_NO_SQL

#include "qsqlcursor.h"
#include "qsqlform.h"

/*!  Constructs a navigator.

*/

QSqlNavigator::QSqlNavigator()
{
}

/*!  Reads the fields from the default form and performs an insert on
  the default cursor.

  \sa defaultCursor() defaultForm()

*/

void QSqlNavigator::insertRecord()
{
    QSqlCursor* cursor = defaultCursor();
    QSqlForm* form = defaultForm();
    if ( !cursor || !form )
	return;
    form->writeFields();
    cursor->insert();
}

/*!  Reads the fields from the default form and performs an update on
the default cursor.

*/

void QSqlNavigator::updateRecord()
{
    QSqlCursor* cursor = defaultCursor();
    QSqlForm* form = defaultForm();
    if ( !cursor || !form )
	return;
    form->writeFields();
    int n = cursor->at();
    cursor->update();
    cursor->select( cursor->filter(), cursor->sort() );
    cursor->seek( n );
}

/*!  Performs a delete on the default cursor and updates the default
form.

*/

void QSqlNavigator::deleteRecord()
{
    QSqlCursor* cursor = defaultCursor();
    QSqlForm* form = defaultForm();
    if ( !cursor || !form )
	return;
    int n = cursor->at();
    if ( cursor->del() ) {
        cursor->select( cursor->filter(), cursor->sort() );
        if ( !cursor->seek( n ) )
            cursor->last();
        cursor->primeEditBuffer();
	QSqlForm* form = defaultForm();    
	if ( form )
	    form->readFields();
    }
}

/*!  Moves the default cursor to the first record and updates the
default form.

*/

void QSqlNavigator::firstRecord()
{
    QSqlCursor* cursor = defaultCursor();
    if ( !cursor )
	return;
    cursor->first();
    cursor->primeEditBuffer();
    QSqlForm* form = defaultForm();    
    if ( form )
	form->readFields();
}

/*!  Moves the default cursor to the last record and updates the
default form.

*/

void QSqlNavigator::lastRecord()
{
    QSqlCursor* cursor = defaultCursor();
    if ( !cursor )
	return;
    cursor->last();
    cursor->primeEditBuffer();
    QSqlForm* form = defaultForm();    
    if ( form )
	form->readFields();
}

/*!  Moves the default cursor to the next record and updates the
default form.

*/

void QSqlNavigator::nextRecord()
{
    QSqlCursor* cursor = defaultCursor();
    if ( !cursor )
	return;
    if( !cursor->next() ) {
        cursor->last();
    }
    cursor->primeEditBuffer();
    QSqlForm* form = defaultForm();    
    if ( form )
	form->readFields();
}

/*!  Moves the default cursor to the previous record and updates the
default form.

*/

void QSqlNavigator::prevRecord()
{
    QSqlCursor* cursor = defaultCursor();
    if ( !cursor )
	return;
    if( !cursor->prev() ) {
        cursor->first();
    }
    cursor->primeEditBuffer();
    QSqlForm* form = defaultForm();    
    if ( form )
	form->readFields();
}

/*!  Clears the default cursor values and clears the default form.

*/

void QSqlNavigator::clearForm()
{
    QSqlCursor* cursor = defaultCursor();
    if ( cursor )
	cursor->editBuffer()->clearValues();
    QSqlForm* form = defaultForm();
    if ( form )
	form->clearValues();
}


/*! Returns a pointer to the default cursor used for navigation, or 0
if there is no default cursor.  The default implementation returns 0.

*/

QSqlCursor* QSqlNavigator::defaultCursor()
{
    return 0;
}


/*! Returns a pointer to the default form used during navigation, or 0
if there is no default form.  The default implementation returns 0.

*/

QSqlForm* QSqlNavigator::defaultForm()
{
    return 0;
}

#endif
