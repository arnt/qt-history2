/****************************************************************************
**
** Implementation of sql navigator classes
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

class QSqlCursorNavigator::QSqlCursorNavigatorPrivate
{
public:
    QSqlCursorNavigatorPrivate() : cur(0) {}
    QString ftr;
    QStringList srt;
    QSqlCursor* cur;
};

/*!
  \class QSqlCursorNavigator qsqlnavigator.h
  \brief The QSqlCursorNavigator class navigates a database cursor.

  \module sql

  This class provides common cursor navigation functionality.  This
  includes saving and applying sorts and filters, refreshing (i.e.,
  re-selecting) the cursor and searching for records within the
  cursor.

  QSqlCursorNavigator is not functional on its own. A variety of
  subclasses provide immediately usable behaviour; this class is a
  pure abstract superclass providing the behaviour that is shared
  among all the concrete SQL navigator classes.

*/

/*!  Constructs a navigator base.

*/

QSqlCursorNavigator::QSqlCursorNavigator()
{
    d = new QSqlCursorNavigatorPrivate();
}


/*! Destroys the object and frees any allocated resources.

*/

QSqlCursorNavigator::~QSqlCursorNavigator()
{
    delete d;
}

void QSqlCursorNavigator::setSort( const QSqlIndex& sort )
{
    setSort( sort.toStringList() );
}

void QSqlCursorNavigator::setSort( const QStringList& sort )
{
    d->srt = sort;
}

QStringList  QSqlCursorNavigator::sort() const
{
    return d->srt;
}

void QSqlCursorNavigator::setFilter( const QString& filter )
{
    d->ftr = filter;
}

QString QSqlCursorNavigator::filter() const
{
    return d->ftr;
}

void QSqlCursorNavigator::setCursor( QSqlCursor* cursor )
{
    d->cur = cursor;
}

/*! Returns a pointer to the default cursor used for navigation, or 0
  if there is no default cursor.

  \sa setCursor()

*/

QSqlCursor* QSqlCursorNavigator::cursor() const
{
    return d->cur;
}


/*! Refreshes the navigator using the default cursor.  If the default
  cursor specifies its own filter and sort, it is refreshed using
  those values.  Otherwise, the navigator's filter and sort are
  applied.

  \sa setFilter() setSort()

*/

void QSqlCursorNavigator::refresh()
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return;
    QString currentFilter = d->ftr;
    QStringList currentSort = cur->sort().toStringList( QString::null, TRUE );
    if ( !currentSort.count() )
	currentSort = d->srt;
    QSqlIndex newSort = QSqlIndex::fromStringList( currentSort, cur );
    cur->select( currentFilter, newSort );
}

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
    QSqlCursor* myCursor = myNavigator.cursor();
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
bool QSqlCursorNavigator::findBuffer( const QSqlIndex& idx, int atHint )
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return FALSE;
    if ( !cur->isActive() )
	return FALSE;
    QSqlRecord* buf = cur->editBuffer();

    bool seekPrimary = (idx.count() ? TRUE : FALSE );

    QApplication::setOverrideCursor( Qt::waitCursor );
    bool indexEquals = FALSE;

    if ( seekPrimary ) {

	indexEquals = FALSE;

	/* check the hint */
	if ( cur->seek( atHint ) )
	    indexEquals = q_index_matches( cur, idx );

	if ( !indexEquals ) {
	    /* check current page */
	    int pageSize = 20;
	    int startIdx = QMAX( atHint - pageSize, 0 );
	    int endIdx = atHint + pageSize;
	    for ( int j = startIdx; j <= endIdx; ++j ) {
		if ( cur->seek( j ) ) {
		    indexEquals = q_index_matches( cur, idx );
		    if ( indexEquals )
			break;
		}
	    }
	}

	if ( !indexEquals && cur->driver()->hasQuerySizeSupport() ) {
	    /* binary search based on record buffer and current sort fields */
	    int lo = 0;
	    int hi = cur->size();
	    int mid;
	    if ( q_compare( buf, cur, cur->sort() ) >= 0 )
		lo = cur->at();
	    while( lo != hi ) {
		mid = lo + (hi - lo) / 2;
		if ( !cur->seek( mid ) )
		    break;
		if ( q_index_matches( cur, idx ) ) {
		    indexEquals = TRUE;
		    break;
		}
		int c = q_compare( buf, cur, cur->sort() );
		if ( c < 0 )
		    hi = mid;
		else if ( c == 0 ) {
		    // found it, but there may be duplicates
		    int at = mid;
		    do {
			mid--;
			if ( !cur->seek( mid ) )
			    break;
			if ( q_index_matches( cur, idx ) ) {
			    indexEquals = TRUE;
			    break;
			}
		    } while ( q_compare( buf, cur, cur->sort() ) == 0 );
		    if ( !indexEquals ) {
			mid = at;
			do {
			    mid++;
			    if ( !cur->seek( mid ) )
				break;
			    if ( q_index_matches( cur, idx ) ) {
				indexEquals = TRUE;
				break;
			    }
			} while ( q_compare( buf, cur, cur->sort() ) == 0 );
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
	    if ( cur->at() != startIdx ) {
		cur->seek( startIdx );
	    }
	    for ( ;; ) {
		indexEquals = FALSE;
		indexEquals = q_index_matches( cur, idx );
		if ( indexEquals )
		    break;
		if ( !cur->next() )
		    break;
	    }
	}
    }
    QApplication::restoreOverrideCursor();
    return indexEquals;
}

class QSqlFormNavigator::QSqlFormNavigatorPrivate
{
public:
    QSqlFormNavigatorPrivate() : frm(0), boundryCheck( TRUE ) {}
    QSqlForm* frm;
    bool boundryCheck;
};


/*!
  \class QSqlFormNavigator qsqlnavigator.h
  \brief The QSqlFormNavigator class navigates a database cursor/form.

  \module sql

  This class provides navigation functionality for a form.  It is used
  by QSqlWidget and QSqlDialog to provide automatic cursor navigation
  when editing/browsing a database cursor.

  QSqlFormNavigator can determine boundry conditions of the cursor (i.e.,
  whether the cursor is on the first or last record) with boundry().
  This can be used, for example, to update widgets according to the
  position of the navigator within the cursor.

  \sa QSqlWidget QSqlDialog QSqlTable

*/

/*! \enum QSqlFormNavigator::Boundry

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

QSqlFormNavigator::QSqlFormNavigator()
    : QSqlCursorNavigator()
{
    d = new QSqlFormNavigatorPrivate();
}


/*! Destroys the object and frees any allocated resources.

*/

QSqlFormNavigator::~QSqlFormNavigator()
{
    delete d;
}

void QSqlFormNavigator::setForm( QSqlForm* form )
{
    d->frm = form;
}

/*! Returns a pointer to the default form used during navigation, or 0
  if there is no default form.  The default implementation returns 0.

*/

QSqlForm* QSqlFormNavigator::form()
{
    return d->frm;
}

/*!  Reads the fields from the default form and performs an insert on
  the default cursor.  Returns 1 if the insert was successfull,
  otherwise 0 is returned.  If an error occurred during the insert
  into the database, handleError() is called.

  \sa cursor() form() handleError()

*/

int QSqlFormNavigator::insertRecord()
{
    QSqlCursor* cur = cursor();
    QSqlForm* frm = form();
    if ( !cur || !frm )
	return 0;
    frm->writeFields();
    int ar = cur->insert();
    if ( !ar || !cur->isActive() )
	handleError( cur->lastError() );
    refresh();
    findBuffer( cur->primaryIndex() );
    updateBoundry();
    return ar;
}

/*!  Reads the fields from the default form and performs an update on
  the default cursor. Returns 1 if the update was successfull,
  otherwise 0 is returned.  If an error occurred during the update on
  the database, handleError() is called.

*/

int QSqlFormNavigator::updateRecord()
{
    QSqlCursor* cur = cursor();
    QSqlForm* frm = form();
    if ( !cur || !frm )
	return 0;
    frm->writeFields();
    int ar = cur->update();
    if ( !ar || !cur->isActive() )
	handleError( cur->lastError() );
    else {
	refresh();
	findBuffer( cur->primaryIndex() );
	updateBoundry();
	frm->readFields();
    }
    return ar;
}

/*!  Performs a delete on the default cursor and updates the default
  form.  Returns 1 if the delete was successfull, otherwise 0 is
  returned.  If an error occurred during the delete from the database,
  handleError() is called.


*/

int QSqlFormNavigator::deleteRecord()
{
    QSqlCursor* cur = cursor();
    QSqlForm* frm = form();
    if ( !cur || !frm )
	return 0;
    int n = cur->at();
    int ar = cur->del();
    if ( ar ) {
	refresh();
	if ( !cur->seek( n ) )
	    lastRecord();
	else
	    updateBoundry();
	cur->primeUpdate();
	QSqlForm* frm = form();
	if ( frm )
	    frm->readFields();
    } else {
	if ( !cur->isActive() )
	    handleError( cur->lastError() );
    }
    return ar;
}

/*!  Moves the default cursor to the first record and updates the
default form.  Returns TRUE if the navigator successfully moved to the
first record, otherwise FALSE is returned.

*/

bool QSqlFormNavigator::firstRecord()
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return FALSE;
    if ( cur->first() ) {
	cur->primeUpdate();
	QSqlForm* frm = form();
	if ( frm )
	    frm->readFields();
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

bool QSqlFormNavigator::lastRecord()
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return FALSE;
    if ( cur->last() ) {
	cur->primeUpdate();
	QSqlForm* frm = form();
	if ( frm )
	    frm->readFields();
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

bool QSqlFormNavigator::nextRecord()
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return FALSE;
    bool b = cur->next();
    if( !b )
	cur->last();
    cur->primeUpdate();
    QSqlForm* frm = form();
    if ( frm )
	frm->readFields();
    updateBoundry();
    return b;
}

/*!  Moves the default cursor to the previous record and updates the
default form.  Returns TRUE if the navigator successfully moved to the
previous record.  Otherwise, the navigator moves the default cursor to
the first record and FALSE is returned.

*/

bool QSqlFormNavigator::prevRecord()
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return FALSE;
    bool b = cur->prev();
    if( !b )
	cur->first();
    cur->primeUpdate();
    QSqlForm* frm = form();
    if ( frm )
	frm->readFields();
    updateBoundry();
    return b;
}

/*!  Clears the default cursor values and clears the widgets in the
default form.

*/

void QSqlFormNavigator::clearForm()
{
    QSqlCursor* cur = cursor();
    if ( cur )
	cur->editBuffer()->clearValues();
    QSqlForm* frm = form();
    if ( frm )
	frm->clearValues();
}

/*!  Virtual function which is called when an error has occurred on
  the default cursor.  The default implementation does nothing.

*/

void QSqlFormNavigator::handleError( const QSqlError& )
{
}

/*! Returns an enum indicating the boundry status of the navigator.
This is done by moving the default cursor and checking the position,
however the current default form values will not be altered.  After
checking for the boundry, the cursor is moved back to its former
position.

  \sa Boundry
*/
QSqlFormNavigator::Boundry QSqlFormNavigator::boundry()
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return Unknown;
    if ( !cur->isActive() )
	return Unknown;
    if ( !cur->isValid() ) {
	if ( cur->at() == QSqlResult::BeforeFirst )
	    return BeforeBeginning;
	if ( cur->at() == QSqlResult::AfterLast )
	    return AfterEnd;
	return Unknown;
    }
    if ( cur->at() == 0 )
	return Beginning;
    // otherwise...
    int currentAt = cur->at();
    Boundry b = None;
    if ( !cur->prev() )
	b = Beginning;
    else
	cur->seek( currentAt );
    if ( b == None && !cur->next() )
	b = End;
    cur->seek( currentAt );
    return b;
}

void QSqlFormNavigator::setBoundryChecking( bool active )
{
    d->boundryCheck = active;
}

bool QSqlFormNavigator::boundryChecking() const
{
    return d->boundryCheck;
}

/*! If boundryChecking() is TRUE, checks the boundry of the current
default cursor and calls virtual 'emit' functions which derived
classes can reimplement to emit signals.
*/

void QSqlFormNavigator::updateBoundry()
{
    if ( d->boundryCheck ) {
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

void QSqlFormNavigator::emitFirstRecordAvailable( bool )
{
}

void QSqlFormNavigator::emitLastRecordAvailable( bool )
{
}

void QSqlFormNavigator::emitNextRecordAvailable( bool )
{
}

void QSqlFormNavigator::emitPrevRecordAvailable( bool )
{
}

#endif
