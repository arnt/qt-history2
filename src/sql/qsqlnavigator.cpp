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
    QSqlCursorNavigatorPrivate() : cur( 0 ), autoDelete( FALSE ), boundryCheck( TRUE ) {}
    QString ftr;
    QStringList srt;
    QSqlCursor* cur;
    bool autoDelete;
    bool boundryCheck;
};

/*! \enum QSqlCursorNavigator::Boundry

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


/*!
  \class QSqlCursorNavigator qsqlnavigator.h
  \brief The QSqlCursorNavigator class navigates a database cursor.

  \module sql

  This class provides common cursor navigation functionality.  This
  includes saving and applying sorts and filters, refreshing (i.e.,
  re-selecting) the cursor and searching for records within the
  cursor.

  QSqlCursorNavigator can determine boundry conditions of the cursor
  (i.e., whether the cursor is on the first or last record) with
  boundry().

*/

/*!  Constructs a curosr navigator.

*/

QSqlCursorNavigator::QSqlCursorNavigator()
{
    d = new QSqlCursorNavigatorPrivate();
}


/*! Destroys the object and frees any allocated resources.

*/

QSqlCursorNavigator::~QSqlCursorNavigator()
{
    if ( d->autoDelete )
	delete d->cur;
    delete d;
}

/*! Sets the navigator's sort to the index \a sort.  To apply the new
  sort, use refresh().

 */

void QSqlCursorNavigator::setSort( const QSqlIndex& sort )
{
    setSort( sort.toStringList() );
}

/*! Sets the navigator's sort to the stringlist \a sort.  To apply the
  new sort, use refresh().

 */

void QSqlCursorNavigator::setSort( const QStringList& sort )
{
    d->srt = sort;
}

/*! Returns the current sort, or an empty stringlist if there is none.

*/

QStringList  QSqlCursorNavigator::sort() const
{
    return d->srt;
}

/*! Sets the navigator's filter to the string \a filter.  To apply the
  new filter, use refresh().

*/

void QSqlCursorNavigator::setFilter( const QString& filter )
{
    d->ftr = filter;
}

/*! Returns the current filter, or an empty string if there is none.

*/

QString QSqlCursorNavigator::filter() const
{
    return d->ftr;
}

/*! Sets the default cursor used by the navigator to \a cursor.  If \a
  autoDelete is TRUE (the default is FALSE), the navigator takes
  ownership of the \a cursor pointer, which will be deleted when the
  navigator is destroyed, or when setCursor() is called again. To
  activate the \a cursor use refresh().

  \sa cursor()

*/

void QSqlCursorNavigator::setCursor( QSqlCursor* cursor, bool autoDelete )
{
    if ( d->autoDelete )
	delete d->cur;
    d->cur = cursor;
    d->autoDelete = autoDelete;
}

/*! Returns a pointer to the default cursor used for navigation, or 0
  if there is no default cursor.

  \sa setCursor()

*/

QSqlCursor* QSqlCursorNavigator::cursor() const
{
    return d->cur;
}


/*! Refreshes the navigator using the default cursor.  The navigator's
  filter and sort are applied.

  \sa setFilter() setSort()

*/

void QSqlCursorNavigator::refresh()
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return;
    QString currentFilter = d->ftr;
    QStringList currentSort = d->srt;
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
 (currently only uses first field) ##
*/

int q_compare( const QSqlRecord* buf1, const QSqlRecord* buf2, const QSqlIndex& idx )
{
    int cmp = 0;

    //    QString s1, s2; //##

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
	//	s1 = f1->value().toString().simplifyWhiteSpace() + ";"; ### what is this?
	//	s2 = buf2->value( fn ).toString().simplifyWhiteSpace() + ";";
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
    if ( !idx.count() )
	return FALSE;

    QSqlRecord* buf = cur->editBuffer();

    QApplication::setOverrideCursor( Qt::waitCursor );
    bool indexEquals = FALSE;
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

    if ( !indexEquals && cur->driver()->hasQuerySizeSupport() && cur->sort().count() ) {
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
    QApplication::restoreOverrideCursor();
    currentChanged( cur );
    return indexEquals;
}

/*! Virtual function called whenever the position of the current
  cursor had changed.  The \a record parameter points to the cursor
  buffer.

*/

void QSqlCursorNavigator::currentChanged( const QSqlRecord* )
{

}

/*! This function is called before a record is inserted using the
  default cursor.  The \a buf parameter points to the edit buffer
  about to be inserted. The default implementation does nothing.

  \sa setCursor()
*/
void QSqlCursorNavigator::beforeInsert( QSqlRecord* )
{
}


/*! This function is called before a record is updated using the
  default cursor.  The \a buf parameter points to the edit buffer
  about to be updated. The default implementation does nothing.

  \sa setCursor()
*/
void QSqlCursorNavigator::beforeUpdate( QSqlRecord* )
{
}

/*! This function is called before a record is deleted using the
  default cursor.  The \a buf parameter points to the edit buffer
  about to be deleted.  The default implementation does nothing.

  \sa setCursor()
*/
void QSqlCursorNavigator::beforeDelete( QSqlRecord* )
{
}

/*! This function is called whenever the cursor changes state.  The \a
  mode parameter describes the change that took place.  The default
  implementation does nothing.
*/
void QSqlCursorNavigator::cursorChanged( QSqlCursor::Mode )
{
}

/*!  Performs an insert on the default cursor.  If there is no default
  cursor, nothing happens and 0 is returned.  Otherwise, returns 1 if
  the insert was successfull, otherwise 0 is returned.  If an error
  occurred during the insert into the database, handleError() is
  called.  If the insert was successfull, the cursor is refreshed and
  relocated to the newly inserted record, and the cursorChanged()
  function is called.

  \sa beforeInsert() cursorChanged() setCursor() handleError()

*/

int QSqlCursorNavigator::insert()
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return 0;
    beforeInsert( cur );
    int ar = cur->insert();
    if ( !ar || !cur->isActive() )
	handleError( cur->lastError() );
    else {
	refresh();
	findBuffer( cur->primaryIndex() );
	updateBoundry();
	cursorChanged( QSqlCursor::Insert );
    }
    return ar;
}

/*!  Performs an update on the default cursor. If there is no default
  cursor, nothing happens and 0 is returned.  Otherwise, returns 1 if
  the update was successfull, otherwise 0 is returned.  If the update
  was successfull, the cursor is refreshed and relocated to the
  updated record, and the cursorChanged() function is called.  If an
  error occurred during the update on the database, handleError() is
  called.

  \sa beforeUpdate() cursorChanged() setCursor() handleError()
*/

int QSqlCursorNavigator::update()
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return 0;
    beforeUpdate( cur );
    int ar = cur->update();
    if ( !ar || !cur->isActive() )
	handleError( cur->lastError() );
    else {
	refresh();
	findBuffer( cur->primaryIndex() );
	updateBoundry();
	cursorChanged( QSqlCursor::Update );
    }
    return ar;
}

/*!  Performs a delete on the default cursor.  If there is no default
  cursor, nothing happens and 0 is returned.  Otherwise, returns 1 if
  the delete was successfull, otherwise 0 is returned.  If the delete
  was successful, the cursor is refreshed, but not relocated, and the
  cursorChanged() function is called. If an error occurred during the
  delete from the database, handleError() is called.

*/

int QSqlCursorNavigator::del()
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return 0;
    beforeDelete( cur );
    int ar = cur->del();
    if ( ar ) {
	refresh();
	updateBoundry();
	cursorChanged( QSqlCursor::Delete );
    } else {
	if ( !cur->isActive() )
	    handleError( cur->lastError() );
    }
    return ar;
}

/*!  Virtual function which is called when an error has occurred on
  the default cursor.  The default implementation does nothing.

*/

void QSqlCursorNavigator::handleError( const QSqlError& )
{
}


/*! If boundryChecking() is TRUE, checks the boundry of the current
  default cursor and calls virtual functions which indicate the
  position of the cursor.
*/

void QSqlCursorNavigator::updateBoundry()
{
    if ( d->boundryCheck ) {
	Boundry bound = boundry();
	switch ( bound ) {
	case Unknown:
	case None:
	    firstRecordAvailable( TRUE );
	    prevRecordAvailable( TRUE );
	    nextRecordAvailable( TRUE );
	    lastRecordAvailable( TRUE );
	    break;

	case BeforeBeginning:
	    firstRecordAvailable( TRUE );
	    prevRecordAvailable( FALSE );
	    nextRecordAvailable( TRUE );
	    lastRecordAvailable( TRUE );
	    break;

	case Beginning:
	    firstRecordAvailable( FALSE );
	    prevRecordAvailable( FALSE );
	    nextRecordAvailable( TRUE );
	    lastRecordAvailable( TRUE );
	    break;

	case End:
	    firstRecordAvailable( TRUE );
	    prevRecordAvailable( TRUE );
	    nextRecordAvailable( FALSE );
	    lastRecordAvailable( FALSE );
	    break;

	case AfterEnd:
	    firstRecordAvailable( TRUE );
	    prevRecordAvailable( TRUE );
	    nextRecordAvailable( FALSE );
	    lastRecordAvailable( TRUE );
	    break;

	}
    }
}

/*!  Moves the default cursor to the first record.  If there is no
  default cursor, nothing happens and 0 is returned.  Otherwise, returns
  TRUE if the navigator successfully moved to the first record,
  otherwise FALSE is returned.

*/

bool QSqlCursorNavigator::first()
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return FALSE;
    if ( cur->first() ) {
	updateBoundry();
	return TRUE;
    }
    updateBoundry();
    return FALSE;
}

/*!  Moves the default cursor to the last record.  If there is no
  default cursor, nothing happens and 0 is returned.  Otherwise,
  returns TRUE if the navigator successfully moved to the last record,
  otherwise FALSE is returned.

*/

bool QSqlCursorNavigator::last()
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return FALSE;
    if ( cur->last() ) {
	updateBoundry();
	return TRUE;
    }
    updateBoundry();
    return FALSE;
}

/*!  Moves the default cursor to the next record.  If there is no
  default cursor, nothing happens and 0 is returned.  Otherwise,
  returns TRUE if the navigator successfully moved to the next record.
  Otherwise, the navigator moves the default cursor to the last record
  and FALSE is returned.

*/

bool QSqlCursorNavigator::next()
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return FALSE;
    bool b = cur->next();
    if( !b )
	cur->last();
    updateBoundry();
    return b;
}

/*!  Moves the default cursor to the previous record.  If there is no
  default cursor, nothing happens and 0 is returned.  Otherwise,
  returns TRUE if the navigator successfully moved to the previous
  record.  Otherwise, the navigator moves the default cursor to the
  first record and FALSE is returned.

*/

bool QSqlCursorNavigator::prev()
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return FALSE;
    bool b = cur->prev();
    if( !b )
	cur->first();
    updateBoundry();
    return b;
}

/*! Returns an enum indicating the boundry status of the navigator.
This is done by moving the default cursor and checking the position,
however the current default form values will not be altered.  After
checking for the boundry, the cursor is moved back to its former
position.

  \sa Boundry
*/
QSqlCursorNavigator::Boundry QSqlCursorNavigator::boundry()
{
    QSqlCursor* cur = cursor();
    if ( !cur || !cur->isActive() )
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

void QSqlCursorNavigator::setBoundryChecking( bool active )
{
    d->boundryCheck = active;
}

bool QSqlCursorNavigator::boundryChecking() const
{
    return d->boundryCheck;
}
/*! \internal
 */
void QSqlCursorNavigator::firstRecordAvailable( bool )
{
}

/*! \internal
 */
void QSqlCursorNavigator::lastRecordAvailable( bool )
{
}

/*! \internal
 */
void QSqlCursorNavigator::nextRecordAvailable( bool )
{
}

/*! \internal
 */
void QSqlCursorNavigator::prevRecordAvailable( bool )
{
}

class QSqlFormNavigator::QSqlFormNavigatorPrivate
{
public:
    QSqlFormNavigatorPrivate() : frm(0) {}
    QSqlForm* frm;
};

/*!
  \class QSqlFormNavigator qsqlnavigator.h
  \brief The QSqlFormNavigator class navigates a database cursor/form.

  \module sql

  This class provides navigation functionality for a SQL form.  It is
  used by QSqlWidget and QSqlDialog to provide automatic cursor
  navigation when editing/browsing a database cursor and form.

  \sa QSqlWidget QSqlDialog

*/

/*!  Constructs a form navigator.

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

/*! Sets the default form to be used by the navigator to \a form.
*/

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

/*!  Reads the fields from the default form into the default cursor
  and performs an insert on the default cursor.  If there is no
  default form, nothing happens, and 0 is returned. Otherwise, returns
  1 if the insert was successfull, otherwise 0 is returned.  If an
  error occurred during the insert into the database, handleError() is
  called.

  \sa QSqlCursorNavigator::insert() cursor() form() handleError()

*/

int QSqlFormNavigator::insert()
{
    QSqlForm* frm = form();
    if ( !frm )
	return 0;
    frm->writeFields();
    int ar = QSqlCursorNavigator::insert();
    return ar;
}

/*!  Reads the fields from the default form into the default cursor
  and performs an update on the default cursor. If there is no default
  form, nothing happens, and 0 is returned.  Otherwise, returns 1 if
  the update was successfull, otherwise 0 is returned.  If an error
  occurred during the update on the database, handleError() is called.

  \sa QSqlCursorNavigator::update() cursor() form() handleError()

*/

int QSqlFormNavigator::update()
{
    QSqlForm* frm = form();
    if ( !frm )
	return 0;
    frm->writeFields();
    int ar = QSqlCursorNavigator::update();
    if ( ar ) {
	cursor()->editBuffer( TRUE );
	frm->readFields();
    }
    return ar;
}

/*!  Performs a delete on the default cursor using the values from the
  default form and updates the default form.  If there is no default
  form, nothing happens and 0 is returned.  Otherwise, returns 1 if
  the delete was successfull, otherwise 0 is returned.  If the delete
  was successful, the cursor is repositioned to the next record. If an
  error occurred during the delete from the database, handleError() is
  called.

  \sa QSqlCursorNavigator::del() cursor() form() handleError()

*/

int QSqlFormNavigator::del()
{
    QSqlCursor* cur = cursor();
    QSqlForm* frm = form();
    if ( !cur || !frm )
	return 0;
    int n = cur->at();
    int ar = QSqlCursorNavigator::del();
    if ( ar ) {
	if ( !cur->seek( n ) )
	    last();
	cur->editBuffer();
	frm->readFields();
    }
    return ar;
}

/*! Causes the default form to read its fields .  If there is no
  default form, nothing happens.

  \sa setForm()

*/

void QSqlFormNavigator::readFields()
{
    if ( form() )
	form()->readFields();
}

/*! Causes the default form to write its fields .  If there is no
  default form, nothing happens.

  \sa setForm()

*/

void QSqlFormNavigator::writeFields()
{
    if ( form() )
	form()->writeFields();
}

/*!  Moves the default cursor to the first record and updates the
  default form.  If there is no default form, nothing happens and 0 is
  returned.  Otherwise, returns TRUE if the navigator successfully
  moved to the first record, otherwise FALSE is returned.

  \sa QSqlCursorNavigator::first()
*/

bool QSqlFormNavigator::first()
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return FALSE;
    bool b = QSqlCursorNavigator::first();
    if ( b ) {
	cur->primeUpdate();
	QSqlForm* frm = form();
	if ( frm )
	    frm->readFields();
    }
    return b;
}

/*!  Moves the default cursor to the last record and updates the
  default form.  If there is no default form, nothing happens and 0 is
  returned.  Otherwise, returns TRUE if the navigator successfully
  moved to the last record, otherwise FALSE is returned.

*/

bool QSqlFormNavigator::last()
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return FALSE;
    bool b = QSqlCursorNavigator::last();
    if ( b ) {
	cur->primeUpdate();
	QSqlForm* frm = form();
	if ( frm )
	    frm->readFields();
    }
    return b;
}

/*!  Moves the default cursor to the next record and updates the
  default form.  If there is no default form, nothing happens and 0 is
  returned.  Otherwise, returns TRUE if the navigator successfully
  moved to the next record.  Otherwise, the navigator moves the
  default cursor to the last record and FALSE is returned.

*/

bool QSqlFormNavigator::next()
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return FALSE;
    bool b = QSqlCursorNavigator::next();
    cur->primeUpdate();
    QSqlForm* frm = form();
    if ( frm )
	frm->readFields();
    return b;
}

/*!  Moves the default cursor to the previous record and updates the
  default form.  If there is no default form, nothing happens and 0 is
  returned.  Otherwise, returns TRUE if the navigator successfully
  moved to the previous record.  Otherwise, the navigator moves the
  default cursor to the first record and FALSE is returned.

*/

bool QSqlFormNavigator::prev()
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return FALSE;
    bool b = QSqlCursorNavigator::prev();
    cur->primeUpdate();
    QSqlForm* frm = form();
    if ( frm )
	frm->readFields();
    return b;
}

/*!  Clears the default cursor values and clears the widgets in the
default form.

*/

void QSqlFormNavigator::clearValues()
{
    QSqlCursor* cur = cursor();
    if ( cur )
	cur->editBuffer()->clearValues();
    QSqlForm* frm = form();
    if ( frm )
	frm->clearValues();
}

#endif
