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
#include "qstring.h"
#include "qstringlist.h"

class QSqlNavigatorBasePrivate
{
public:
    QString ftr;
    QStringList srt;
};


/*!
  \class QSqlNavigatorBase qsqlnavigator.h
  \brief Navigate a database cursor

  \module sql

  This class provides common cursor navigation functionality.  This
  includes saving and applying sorts and filters, refreshing (i.e.,
  re-selecting) the cursor and searching for records within the
  cursor.

  QSqlNavigatorBase is not functional on its own. A variety of
  subclasses provide immediately usable behaviour; this class is a
  pure abstract superclass providing the behaviour that is shared
  among all the concrete SQL navigator classes.

*/

/*!  Constructs a navigator base.

*/

QSqlNavigatorBase::QSqlNavigatorBase()
{
    d = new QSqlNavigatorBasePrivate();
}


/*! Destroys the object and frees any allocated resources.

*/

QSqlNavigatorBase::~QSqlNavigatorBase()
{
    delete d;
}

void QSqlNavigatorBase::setSort( const QSqlIndex& sort )
{
    setSort( sort.toStringList() );
}

void QSqlNavigatorBase::setSort( const QStringList& sort )
{
    d->srt = sort;
}

QStringList  QSqlNavigatorBase::sort() const
{
    return d->srt;
}

void QSqlNavigatorBase::setFilter( const QString& filter )
{
    d->ftr = filter;
}

QString QSqlNavigatorBase::filter() const
{
    return d->ftr;
}

/*! Refreshes the navigator using the default cursor.  If the default
  cursor specifies its own filter and sort, it is refreshed using
  those values.  Otherwise, the navigator's filter and sort are
  applied.

  \sa setFilter() setSort()

*/

void QSqlNavigatorBase::refresh()
{
    QSqlCursor* cursor = defaultCursor();
    if ( !cursor )
	return;
    QString currentFilter = cursor->filter();
    if ( currentFilter.isEmpty() )
	currentFilter = d->ftr;
    QStringList currentSort = cursor->sort().toStringList( QString::null, TRUE );
    if ( !currentSort.count() )
	currentSort = d->srt;
    QSqlIndex newSort = QSqlIndex::fromStringList( currentSort, cursor );
    cursor->select( currentFilter, newSort );
}

/*! \fn QSqlCursor* QSqlNavigatorBase::defaultCursor()
  Returns a pointer to the default cursor used for navigation, or 0
  if there is no default cursor.

*/

/* Returns TRUE if the \a buf field values that correspond to \idx
   match the field values in \a idx.
*/

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

/* Return less than, equal to or greater than 0 if buf1 is less than,
 equal to or greater than buf2 according to fields described in idx
 (currently only uses first field)
*/

int q_compare( const QSqlRecord* buf1, const QSqlRecord* buf2, const QSqlIndex& idx )
{
    int cmp = 0;

    QString s1, s2; //##

    //    for ( uint i = 0; i < idx.count(); ++i ) {
    int i = 0;
	const QString fn( idx.field(i)->name() );
	const QSqlField* f1 = buf1->field( fn );
	bool reverse = FALSE;
	if ( idx.isDescending( i ) )
	     reverse = TRUE;
	if ( f1 ) {
	    switch( f1->type() ) { // ## more types?
	    case QVariant::String:
	    case QVariant::CString:
		if ( f1->value().toString().simplifyWhiteSpace() < buf2->value( fn ).toString().simplifyWhiteSpace() )
		    cmp = -1;
		else if ( f1->value().toString().simplifyWhiteSpace() > buf2->value( fn ).toString().simplifyWhiteSpace() )
		    cmp = 1;
		break;
	    default:
		if ( f1->value().toDouble() < buf2->value( fn ).toDouble() )
		    cmp = -1;
		else if ( f1->value().toDouble() > buf2->value( fn ).toDouble() )
		    cmp = 1;
		break;
	    }
	}
	s1 = f1->value().toString().simplifyWhiteSpace() + ";";
	s2 = buf2->value( fn ).toString().simplifyWhiteSpace() + ";";
	//    }
    if ( reverse ) {
	if ( cmp < 0 )
	    cmp = 1;
	else if ( cmp > 0 )
	    cmp = -1;
    }
    return cmp;

}

/*! Relocates the default cursor to the record matching the cursor's
edit buffer.  Only the field names specified by \a idx are used to
determine an exact match of the cursor to the edit buffer. However,
other fields in the edit buffer are also used during the search,
therefore all fields in the edit buffer should be primed with desired
values for the record being sought.  This function is typically used
to relocate a cursor to the correct position after an insert or
update.  For example:

\code
    QSqlCursor* myCursor = myNavigator.defaultCursor();
    ...
    QSqlRecord* buf = myCursor->primeUpdate();
    buf->setValue( "name", "Dave" );
    buf->setValue( "city", "Oslo" );
    ...
    myCursor->update();  // update current record
    myCursor->select();  // refresh the cursor
    myNavigator.findBuffer( myCursor->primaryIndex() ); // go to the updated record
\endcode

*/

//## possibly add sizeHint parameter
bool QSqlNavigatorBase::findBuffer( const QSqlIndex& idx, int atHint )
{
    QSqlCursor* cursor = defaultCursor();
    if ( !cursor )
	return FALSE;
    if ( !cursor->isActive() )
	return FALSE;
    QSqlRecord* buf = cursor->editBuffer();

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
	    if ( q_compare( buf, cursor, cursor->sort() ) >= 0 )
		lo = cursor->at();
	    while( lo != hi ) {
		mid = lo + (hi - lo) / 2;
		if ( !cursor->seek( mid ) )
		    break;
		if ( q_index_matches( cursor, idx ) ) {
		    indexEquals = TRUE;
		    break;
		}
		int c = q_compare( buf, cursor, cursor->sort() );
		if ( c < 0 )
		    hi = mid;
		else if ( c == 0 ) {
		    // found it, but there may be duplicates
		    int at = mid;
		    do {
			mid--;
			if ( !cursor->seek( mid ) )
			    break;
			if ( q_index_matches( cursor, idx ) ) {
			    indexEquals = TRUE;
			    break;
			}
		    } while ( q_compare( buf, cursor, cursor->sort() ) == 0 );
		    if ( !indexEquals ) {
			mid = at;
			do {
			    mid++;
			    if ( !cursor->seek( mid ) )
				break;
			    if ( q_index_matches( cursor, idx ) ) {
				indexEquals = TRUE;
				break;
			    }
			} while ( q_compare( buf, cursor, cursor->sort() ) == 0 );
		    }
		    break;
		} else if ( c > 0 ) {
		    lo = mid + 1;
		}
	    }
	}

	if ( !indexEquals ) {
	    /* give up, use brute force */
	    int startIdx = 0;
	    if ( cursor->at() != startIdx ) {
		cursor->seek( startIdx );
	    }
	    for ( ;; ) {
		indexEquals = FALSE;
		indexEquals = q_index_matches( cursor, idx );
		if ( indexEquals )
		    break;
		if ( !cursor->next() )
		    break;
	    }
	}
    }
    QApplication::restoreOverrideCursor();
    return indexEquals;
}


/*!
  \class QSqlNavigator qsqlnavigator.h
  \brief Navigate a database cursor/form

  \module sql

  This class provides navigation functionality for a form.  It is used
  by QSqlWidget and QSqlDialog to provide automatic cursor navigation
  when editing/browsing a database cursor.

  QSqlNavigator can determine boundry conditions of the cursor (i.e.,
  whether the cursor is on the first or last record) with boundry().
  This can be used, for example, to update widgets according to the
  position of the navigator within the cursor.

  \sa QSqlWidget QSqlDialog QSqlTable

*/

/*! \enum QSqlNavigator::Boundry

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
    : QSqlNavigatorBase(), boundryCheck( TRUE )
{
}


/*! Destroys the object and frees any allocated resources.

*/

QSqlNavigator::~QSqlNavigator()
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
    refresh();
    findBuffer( cursor->primaryIndex() );
    updateBoundry();
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
    int ar = cursor->update();
    if ( !ar || !cursor->isActive() )
	handleError( cursor->lastError() );
    else {
	refresh();
	findBuffer( cursor->primaryIndex() );
	updateBoundry();
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
	refresh();
	if ( !cursor->seek( n ) )
	    lastRecord();
	else
	    updateBoundry();
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

#endif
