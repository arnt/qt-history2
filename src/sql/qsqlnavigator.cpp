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

#include "qapplication.h"
#include "qsqlcursor.h"
#include "qsqlresult.h"
#include "qsqlform.h"
#include "qsqldriver.h"

/*!
  \class QSqlNavigator qsqlnavigator.h
  \brief Navigate a database cursor/form

  \module sql

  This class //###

*/

/*! \enum Boundry

  This enum type describes where the navigator is currently positioned.

  The currently defined values are:
  <ul>

  <li> \c Unknown - the boundry cannot be determined (usually because
  there is no default cursor).

  <li> \c None - the navigator is not positioned on a boundry.

  <li> \c BeforeBeginning - the navigator is positioned before the
  beginning of the available records.

  <li> \c Beginning - the navigator is positioned at the beginning of
  the available records.

  <li> \c End - the navigator is positioned at the end of
  the available records.

  <li> \c AfterEnd - the navigator is positioned after the end of the
  available records.

  </ul>
*/

/*!  Constructs a navigator.

*/

QSqlNavigator::QSqlNavigator()
    : boundryCheck( TRUE )
{
}

/*!  Reads the fields from the default form and performs an insert on
  the default cursor.  Returns 1 if the insert was successfull,
  otherwise 0 is returned.  If an error occurred during the insert
  into the database, handleError() is called.

  \sa defaultCursor() defaultForm() handleError()

*/

int QSqlNavigator::insertRecord()
{
    QSqlCursor* cursor = defaultCursor();
    QSqlForm* form = defaultForm();
    if ( !cursor || !form )
	return 0;
    form->writeFields();
    int ar = cursor->insert();
    if ( !ar || !cursor->isActive() )
	handleError( cursor->lastError() );
    cursor->select();
    QSqlNavigator::relocate( cursor, cursor->editBuffer(), cursor->primaryIndex(), 0 );
    return ar;
}

/*!  Reads the fields from the default form and performs an update on
  the default cursor. Returns 1 if the update was successfull,
  otherwise 0 is returned.  If an error occurred during the update on
  the database, handleError() is called.

*/

int QSqlNavigator::updateRecord()
{
    QSqlCursor* cursor = defaultCursor();
    QSqlForm* form = defaultForm();
    if ( !cursor || !form )
	return 0;
    form->writeFields();
    int n = cursor->at();
    int ar = cursor->update();
    if ( !ar || !cursor->isActive() )
	handleError( cursor->lastError() );
    else {
	cursor->select();
	QSqlNavigator::relocate( cursor, cursor->editBuffer(), cursor->primaryIndex(), n );
	form->readFields();
    }
    return ar;
}

/*!  Performs a delete on the default cursor and updates the default
  form.  Returns 1 if the delete was successfull, otherwise 0 is
  returned.  If an error occurred during the delete from the database,
  handleError() is called.


*/

int QSqlNavigator::deleteRecord()
{
    QSqlCursor* cursor = defaultCursor();
    QSqlForm* form = defaultForm();
    if ( !cursor || !form )
	return 0;
    int n = cursor->at();
    int ar = cursor->del();
    if ( ar ) {
	cursor->select();
	if ( !cursor->seek( n ) )
	    cursor->last();
	cursor->primeUpdate();
	QSqlForm* form = defaultForm();
	if ( form )
	    form->readFields();
    } else {
	if ( !cursor->isActive() )
	    handleError( cursor->lastError() );
    }
    return ar;
}

/*!  Moves the default cursor to the first record and updates the
default form.  Returns TRUE if the navigator successfully moved to the
first record, otherwise FALSE is returned.

*/

bool QSqlNavigator::firstRecord()
{
    QSqlCursor* cursor = defaultCursor();
    if ( !cursor )
	return FALSE;
    if ( cursor->first() ) {
	cursor->primeUpdate();
	QSqlForm* form = defaultForm();
	if ( form )
	    form->readFields();
	updateBoundry();
	return TRUE;
    }
    updateBoundry();
    return FALSE;
}

/*!  Moves the default cursor to the last record and updates the
default form.  Returns TRUE if the navigator successfully moved to the
last record, otherwise FALSE is returned.

*/

bool QSqlNavigator::lastRecord()
{
    QSqlCursor* cursor = defaultCursor();
    if ( !cursor )
	return FALSE;
    if ( cursor->last() ) {
	cursor->primeUpdate();
	QSqlForm* form = defaultForm();
	if ( form )
	    form->readFields();
	updateBoundry();
	return TRUE;
    }
    updateBoundry();
    return FALSE;
}

/*!  Moves the default cursor to the next record and updates the
default form.  Returns TRUE if the navigator successfully moved to the
next record.  Otherwise, the navigator moves the default cursor to the
last record and FALSE is returned.

*/

bool QSqlNavigator::nextRecord()
{
    QSqlCursor* cursor = defaultCursor();
    if ( !cursor )
	return FALSE;
    bool b = cursor->next();
    if( !b )
	cursor->last();
    cursor->primeUpdate();
    QSqlForm* form = defaultForm();
    if ( form )
	form->readFields();
    updateBoundry();
    return b;
}

/*!  Moves the default cursor to the previous record and updates the
default form.  Returns TRUE if the navigator successfully moved to the
previous record.  Otherwise, the navigator moves the default cursor to
the first record and FALSE is returned.

*/

bool QSqlNavigator::prevRecord()
{
    QSqlCursor* cursor = defaultCursor();
    if ( !cursor )
	return FALSE;
    bool b = cursor->prev();
    if( !b )
	cursor->first();
    cursor->primeUpdate();
    QSqlForm* form = defaultForm();
    if ( form )
	form->readFields();
    updateBoundry();
    return b;
}

/*!  Clears the default cursor values and clears the widgets in the
default form.

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


/*!  Virtual function which is called when an error has occurred on
  the default cursor.  The default implementation does nothing.

*/

void QSqlNavigator::handleError( const QSqlError& )
{
}

/*! Returns an enum indicating the boundry status of the navigator.
This is done by moving the default cursor and checking the position,
however the current default form values will not be altered.  After
checking for the boundry, the cursor is moved back to its former
position.

  \sa Boundry
*/
QSqlNavigator::Boundry QSqlNavigator::boundry()
{
    QSqlCursor* cursor = defaultCursor();
    if ( !cursor )
	return Unknown;
    if ( !cursor->isActive() )
	return Unknown;
    if ( !cursor->isValid() ) {
	if ( cursor->at() == QSqlResult::BeforeFirst )
	    return BeforeBeginning;
	if ( cursor->at() == QSqlResult::AfterLast )
	    return AfterEnd;
	return Unknown;
    }
    if ( cursor->at() == 0 )
	return Beginning;
    // otherwise...
    int currentAt = cursor->at();
    Boundry b = None;
    if ( !cursor->prev() )
	b = Beginning;
    else
	cursor->seek( currentAt );
    if ( b == None && !cursor->next() )
	b = End;
    cursor->seek( currentAt );
    return b;
}

void QSqlNavigator::setBoundryChecking( bool active )
{
    boundryCheck = active;
}

bool QSqlNavigator::boundryChecking() const
{
    return boundryCheck;
}

/*! If boundryChecking() is TRUE, checks the boundry of the current
default cursor and calls virtual 'emit' functions which derived
classes can reimplement to emit signals.
*/

void QSqlNavigator::updateBoundry()
{
    if ( boundryCheck ) {
	Boundry bound = boundry();
	switch ( bound ) {
	case Unknown:
	case None:
	    emitFirstRecordAvailable( TRUE );
	    emitPrevRecordAvailable( TRUE );
	    emitNextRecordAvailable( TRUE );
	    emitLastRecordAvailable( TRUE );
	    break;

	case BeforeBeginning:
	    emitFirstRecordAvailable( TRUE );
	    emitPrevRecordAvailable( FALSE );
	    emitNextRecordAvailable( TRUE );
	    emitLastRecordAvailable( TRUE );
	    break;

	case Beginning:
	    emitFirstRecordAvailable( FALSE );
	    emitPrevRecordAvailable( FALSE );
	    emitNextRecordAvailable( TRUE );
	    emitLastRecordAvailable( TRUE );
	    break;

	case End:
	    emitFirstRecordAvailable( TRUE );
	    emitPrevRecordAvailable( TRUE );
	    emitNextRecordAvailable( FALSE );
	    emitLastRecordAvailable( FALSE );
	    break;

	case AfterEnd:
	    emitFirstRecordAvailable( TRUE );
	    emitPrevRecordAvailable( TRUE );
	    emitNextRecordAvailable( FALSE );
	    emitLastRecordAvailable( TRUE );
	    break;

	}
    }
}

void QSqlNavigator::setSort( const QSqlIndex& sort )
{
    QSqlCursor* cursor = defaultCursor();
    if ( cursor )
	cursor->setSort( sort );
}

void QSqlNavigator::setSort( const QStringList& sort )
{
    QSqlCursor* cursor = defaultCursor();
    if ( cursor )
	cursor->setSort( QSqlIndex::fromStringList( sort, cursor ) );
}

QStringList  QSqlNavigator::sort() const
{
    const QSqlCursor* cursor = ((QSqlNavigator*)this)->defaultCursor();
    if ( cursor )
	return cursor->sort().toStringList();
    return QStringList();
}

void QSqlNavigator::setFilter( const QString& filter )
{
    QSqlCursor* cursor = defaultCursor();
    if ( cursor )
	cursor->setFilter( filter );
}

QString QSqlNavigator::filter() const
{
    const QSqlCursor* cursor = ((QSqlNavigator*)this)->defaultCursor();
    if ( cursor )
	return cursor->filter();
    return QString::null;
}


void QSqlNavigator::emitFirstRecordAvailable( bool )
{
}

void QSqlNavigator::emitLastRecordAvailable( bool )
{
}

void QSqlNavigator::emitNextRecordAvailable( bool )
{
}

void QSqlNavigator::emitPrevRecordAvailable( bool )
{
}

bool q_index_matches( const QSqlRecord* buf, const QSqlIndex& idx )
{
    bool indexEquals = FALSE;
    for ( uint i = 0; i < idx.count(); ++i ) {
	const QString fn( idx.field(i)->name() );
	if ( idx.field(i)->value() == buf->value( fn ) )
	    indexEquals = TRUE;
	else {
	    indexEquals = FALSE;
	    break;
	}
    }
    return indexEquals;
}

bool q_less( const QSqlRecord* buf1, const QSqlRecord* buf2, const QSqlIndex& idx )
{
    bool lessThan = FALSE;
    for ( uint i = 0; i < idx.count(); ++i ) {
	const QString fn( idx.field(i)->name() );
	const QSqlField* f1 = buf1->field( fn );
	if ( f1 ) {
	    switch( f1->type() ) { // ## more types?
	    case QVariant::String:
	    case QVariant::CString:
		if ( f1->value().toString() < buf2->value( fn ).toString() )
		    lessThan = TRUE;
		else
		    lessThan = FALSE;
		break;
	    default:
		if ( f1->value().toDouble() < buf2->value( fn ).toDouble() )
		    lessThan = TRUE;
		else
		    lessThan = FALSE;
	    }
	}
	if ( lessThan == FALSE )
	    break;
    }
    return lessThan;
}

bool QSqlNavigator::relocate( QSqlCursor* cursor, const QSqlRecord* buf, const QSqlIndex& idx, int atHint )
{
    if ( !cursor )
	return FALSE;
    if ( !cursor->isActive() )
	return FALSE;

    bool seekPrimary = (idx.count() ? TRUE : FALSE );

    QApplication::setOverrideCursor( Qt::waitCursor );
    bool indexEquals = FALSE;

    if ( seekPrimary ) {

	indexEquals = FALSE;

	/* check the hint */
	if ( cursor->seek( atHint ) )
	    indexEquals = q_index_matches( cursor, idx );

	if ( !indexEquals ) {
	    /* check current page */
	    int pageSize = 20;
	    int startIdx = QMAX( atHint - pageSize, 0 );
	    int endIdx = atHint + pageSize;
	    for ( int j = startIdx; j <= endIdx; ++j ) {
		if ( cursor->seek( j ) ) {
		    indexEquals = q_index_matches( cursor, idx );
		    if ( indexEquals )
			break;
		}
	    }
	}

	if ( !indexEquals && cursor->driver()->hasQuerySizeSupport() ) {
	    /* binary search based on record buffer and current sort fields */
	    int lo = 0;
	    int hi = cursor->size();
	    int mid;
	    if ( !q_less( buf, cursor, cursor->sort() ) )
		lo = cursor->at();
	    while( lo != hi ) {
		mid = lo + (hi - lo) / 2;
		if ( !cursor->seek( mid ) )
		    break;
		if ( q_index_matches( cursor, idx ) ) {
		    indexEquals = TRUE;
		    break;
		}
		if ( q_less( buf, cursor, cursor->sort() ) )
		    hi = mid;
		else
		    lo = mid + 1;
	    }
	}

	if ( !indexEquals ) {
	    /* give up, use brute force */

	    int startIdx = 0;
	    bool reverse = FALSE;
	    if ( q_less( buf, cursor, cursor->sort() ) ) {
		reverse = TRUE;
		startIdx = cursor->at();
	    } else {
		startIdx = cursor->at();
	    }

	    if ( cursor->at() != startIdx )
		cursor->seek( startIdx );
	    for ( ;; ) {
		indexEquals = FALSE;
		indexEquals = q_index_matches( cursor, idx );
		if ( indexEquals )
		    break;
		if ( !reverse ) {
		    if ( !cursor->next() )
			break;
		} else {
		    if ( !cursor->prev() )
			break;
		}
	    }
	}
    }
    QApplication::restoreOverrideCursor();
    return indexEquals;
}

#endif
