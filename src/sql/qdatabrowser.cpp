/****************************************************************************
**
** Implementation of QDataBrowser class
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

#include "qdatabrowser.h"

#ifndef QT_NO_SQL

#include "qsqlform.h"
#include "qsqlmanager_p.h"
#include "qsqlresult.h"

class QDataBrowser::QDataBrowserPrivate
{
public:
    QDataBrowserPrivate() : boundaryCheck( TRUE ), readOnly( FALSE ) {}
    QSqlCursorManager cur;
    QSqlFormManager frm;
    QDataManager dat;
    bool boundaryCheck;
    bool readOnly;
};

/*!

  \class QDataBrowser qdatabrowser.h
  \brief This class provides data manipulation and navigation for data entry forms.

  \module sql

  This class is used to manipulate and navigate data entry forms.  A
  high-level API is provided to navigate through data records in a
  cursor, insert, update and delete records, and refresh data in the
  display.

  Convenient signals and slots are provided to navigate the cursor
  (see firstRecord(), lastRecord(), prevRecord(), nextRecord()), to
  edit records (see insert(), update(), delete()), and to update the
  display according to the cursor's current position (see
  firstRecordAvailable(), lastRecordAvailable(),
  nextRecordAvailable(), prevRecordAvailable()).

*/

/*! \enum QDataBrowser::Boundary

  This enum type describes where the browser is currently positioned.

  The currently defined values are:
  <ul>

  <li> \c Unknown - the boundary cannot be determined (usually because
  there is no default cursor, or the default cursor is not active).

  <li> \c None - the browser is not positioned on a boundary.

  <li> \c BeforeBeginning - the browser is positioned before the
  beginning of the available records.

  <li> \c Beginning - the browser is positioned at the beginning of
  the available records.

  <li> \c End - the browser is positioned at the end of
  the available records.

  <li> \c AfterEnd - the browser is positioned after the end of the
  available records.

  </ul>
*/

QDataBrowser::QDataBrowser( QWidget *parent, const char *name, WFlags fl )
    : QWidget( parent, name, fl )
{
    d = new QDataBrowserPrivate();
    d->dat.setMode( QSql::Update );
}

/*! Destroys the object and frees any allocated resources.

*/

QDataBrowser::~QDataBrowser()
{
    delete d;
}


/*! Returns an enum indicating the boundary status of the browser.

  This is achieved by moving the default cursor and checking the
  position, however the current default form values will not be altered.
  After checking for the boundary, the cursor is moved back to its
  former position.

  \sa Boundary
*/

QDataBrowser::Boundary QDataBrowser::boundary()
{
    QSqlCursor* cur = d->cur.cursor();
    if ( !cur || !cur->isActive() )
	return Unknown;
    if ( !cur->isValid() ) {
	if ( cur->at() == QSql::BeforeFirst )
	    return BeforeBeginning;
	if ( cur->at() == QSql::AfterLast )
	    return AfterEnd;
	return Unknown;
    }
    if ( cur->at() == 0 )
	return Beginning;
    int currentAt = cur->at();
    Boundary b = None;
    if ( !cur->prev() )
	b = Beginning;
    else
	cur->seek( currentAt );
    if ( b == None && !cur->next() )
	b = End;
    cur->seek( currentAt );
    return b;
}


/*! Sets boundary checking to \a active.

    When boundary checking is on, i.e. \a active is TRUE (the default),
    signals are emitted indicating the current position of the default
    cursor.

  \sa boundaryChecking()
*/

void QDataBrowser::setBoundaryChecking( bool active )
{
    d->boundaryCheck = active;
}

/*! Returns TRUE if boundary checking is enabled (the default),
  otherwise returns FALSE.

  \sa setBoundaryChecking()

*/

bool QDataBrowser::boundaryChecking() const
{
    return d->boundaryCheck;
}

/*! Sets the browser's sort to the index \a sort.  To apply the new
  sort, use refresh().

*/

void QDataBrowser::setSort( const QSqlIndex& sort )
{
    d->cur.setSort( sort );
}


/*! Sets the browser's sort to the string list \a sort.  To apply the new
  sort, use refresh().

*/

void QDataBrowser::setSort( const QStringList& sort )
{
    d->cur.setSort( sort );
}

/*! Returns the current sort, or an empty stringlist if there is none.

*/

QStringList QDataBrowser::sort() const
{
    return d->cur.sort();
}


/*! Sets the browser's filter to the string \a filter.  To apply the
  new filter, use refresh(). A filter string is an SQL WHERE clause
  without the WHERE keyword.

*/

void QDataBrowser::setFilter( const QString& filter )
{
    d->cur.setFilter( filter );
}


/*! Returns the current filter (WHERE clause), or an empty string if
    no filter has been set.

*/

QString QDataBrowser::filter() const
{
    return d->cur.filter();
}


/*! Sets the default cursor used by the browser to \a cursor.  If \a
  autoDelete is TRUE (the default is FALSE), the browser takes
  ownership of the \a cursor pointer, which will be deleted when the
  browser is destroyed, or when setCursor() is called again. To
  activate the \a cursor use refresh().  The cursor's edit buffer is
  used in the default form to edit/browse records.

  \sa cursor() form() setForm()

*/

void QDataBrowser::setCursor( QSqlCursor* cursor, bool autoDelete )
{
    d->cur.setCursor( cursor, autoDelete );
    d->frm.setRecord( cursor->editBuffer() );
    if ( cursor->isReadOnly() )
	setReadOnly( TRUE );

}


/*! Sets the default cursor used by the browser to \a cursor. This
   function is a wrapper for setCursor() and is provided purely for
   consistency with sqlCursor(). We recommend using setCursor()
   directly.

  \sa sqlCursor()
*/

void QDataBrowser::setSqlCursor( QSqlCursor* cursor, bool autoDelete )
{
    setCursor( cursor, autoDelete );
}

/*! Returns a pointer to the default cursor used for navigation, or 0
  if there is no default cursor.

  \sa setCursor()

*/

QSqlCursor* QDataBrowser::sqlCursor() const
{
    return d->cur.cursor();
}


/*!  Sets the borwser's default form to \a form.  The cursor and all
  navigation and data manipulation functions that the browser provides
  become available to the \a form.

*/

void QDataBrowser::setForm( QSqlForm* form )
{
    d->frm.setForm( form );
}


/*! Returns a pointer to the browser's default form or 0 if no form
   has been set.

*/

QSqlForm* QDataBrowser::form()
{
    return d->frm.form();
}

/*!  Sets the read only property of the browser to \a active.
  Read-only browsers will not perform database edits.

*/

void QDataBrowser::setReadOnly( bool active )
{
    d->readOnly = active;
}

/*! Returns TRUE if the browser is read-only, otherwise FALSE is
  returned.
*/

bool QDataBrowser::isReadOnly() const
{
    return d->readOnly;
}

/*! If \a confirm is TRUE, all edit operations (inserts, updates and
  deletes) must be confirmed by the user.  If \a confirm is FALSE (the
  default), all edits are posted to the database immediately.

*/
void QDataBrowser::setConfirmEdits( bool confirm )
{
    d->dat.setConfirmEdits( confirm );
}

/*! If \a confirm is TRUE, all inserts must be confirmed by the user.
  If \a confirm is FALSE (the default), all edits are posted to the
  database immediately.

*/

void QDataBrowser::setConfirmInsert( bool confirm )
{
    d->dat.setConfirmInsert( confirm );
}

/*! If \a confirm is TRUE, all updates must be confirmed by the user.
  If \a confirm is FALSE (the default), all edits are posted to the
  database immediately.

*/

void QDataBrowser::setConfirmUpdate( bool confirm )
{
    d->dat.setConfirmUpdate( confirm );
}

/*! If \a confirm is TRUE, all deletes must be confirmed by the user.
  If \a confirm is FALSE (the default), all edits are posted to the
  database immediately.

*/

void QDataBrowser::setConfirmDelete( bool confirm )
{
    d->dat.setConfirmDelete( confirm );
}

/*! Returns TRUE if the browser confirms all edit operations (inserts,
  updates and deletes), otherwise returns FALSE.
*/

bool QDataBrowser::confirmEdits() const
{
    return ( d->dat.confirmEdits() );
}

/*! Returns TRUE if the browser confirms inserts, otherwise returns
  FALSE.
*/

bool QDataBrowser::confirmInsert() const
{
    return ( d->dat.confirmInsert() );
}

/*! Returns TRUE if the browser confirms updates, otherwise returns
  FALSE.
*/

bool QDataBrowser::confirmUpdate() const
{
    return ( d->dat.confirmUpdate() );
}

/*! Returns TRUE if the browser confirms deletes, otherwise returns
  FALSE.
*/

bool QDataBrowser::confirmDelete() const
{
    return ( d->dat.confirmDelete() );
}

/*! If \a confirm is TRUE, all cancels must be confirmed by the user
  through a message box.  If \a confirm is FALSE (the default), all
  cancels occur immediately.
*/

void QDataBrowser::setConfirmCancels( bool confirm )
{
    d->dat.setConfirmCancels( confirm );
}

/*! Returns TRUE if the browser confirms cancels, otherwise returns FALSE.
*/

bool QDataBrowser::confirmCancels() const
{
    return d->dat.confirmCancels();
}

/*!  Sets the autoEdit property of the browser to \a autoEdit. The
  default is TRUE. When the user begins an insert or update on a form
  there are two possible outcomes when they navigate to another record:
  <ol>
  <li> the insert or update is is performed -- this occurs if autoEdit
  is TRUE
  <li> the insert or update is abandoned -- this occurs if autoEdit is
  FALSE
  </ol>

*/

void QDataBrowser::setAutoEdit( bool autoEdit )
{
    d->dat.setAutoEdit( autoEdit );
}


/*! Returns TRUE if the autoEdit property is on, otherwise returns FALSE.

*/

bool QDataBrowser::autoEdit() const
{
    return d->dat.autoEdit();
}


/*! \fn void QDataBrowser::firstRecordAvailable( bool available )

  This signal is emitted whenever the position of the cursor changes.
  The \a available parameter indicates whether or not the first record
  in the default cursor is available.

*/

/*! void QDataBrowser::lastRecordAvailable( bool available )

  This signal is emitted whenever the position of the cursor
  changes.  The \a available parameter indicates whether or not the
  last record in the default cursor is available.

*/

/*! \fn void QDataBrowser::nextRecordAvailable( bool available )

  This signal is emitted whenever the position of the cursor
  changes.  The \a available parameter indicates whether or not the
  next record in the default cursor is available.

*/


/*! \fn void QDataBrowser::prevRecordAvailable( bool available )

  This signal is emitted whenever the position of the cursor
  changes.  The \a available parameter indicates whether or not the
  next record in the default cursor is available.

*/


/*! \fn void QDataBrowser::currentChanged( const QSqlRecord* record )

  This signal is emitted whenever the current cursor position has
  changed.  The \a record parameter points to the contents of the
  current cursor's record.

*/


/*! \fn void QDataBrowser::primeInsert( QSqlRecord* buf )

  This signal is emitted when the QDataBrowser enters insertion mode.
  The \a buf parameter points to the record buffer that is to be
  inserted. Connect to this signal to, for example, prime the record
  buffer with default data values, auto-numbered fields etc. (Note that
  QSqlCursor::primeInsert() is \e not called on the default cursor, as
  this would corrupt values in the form.)

  \sa insert()

*/


/*! \fn void QDataBrowser::primeUpdate( QSqlRecord* buf )

  This signal is emitted before the edit buffer is updated in the
  database by the browser.  Note that during naviagtion ( first(),
  last(), next(), prev()), each record that is shown in the default
  form is primed for update.  The \a buf parameter points to the
  record buffer being updated. (Note that QSqlCursor::primeUpdate() is
  \e not called on the default cursor, as this would corrupt values in
  the form.) Connect to this signal in order to, for example, keep
  track of which records have been updated, perhaps for auditing
  purposes.

  \sa update()

*/

/*! \fn void QDataBrowser::primeDelete( QSqlRecord* buf )

  This signal is emitted before the edit buffer is deleted from the
  database by the browser.  The \a buf parameter points to the record
  buffer being deleted. (Note that QSqlCursor::primeDelete() is \e
  not called on the default cursor, as this would corrupt values in
  the form.)  Connect to this signal in order to, for example, save a
  copy of the deleted record for auditing purposes.

  \sa delete()

*/


/*! \fn void QDataBrowser::cursorChanged( QSqlCursor::Mode mode )

  This signal is emitted whenever the cursor record was changed due to
  navigation.  The \a mode parameter is the edit that just took place,
  e.g. Insert, Update or Delete. See \l QSqlCursor::Mode.

*/


/*! Refreshes the browser's data using the default cursor.  The browser's
  current filter and sort are applied if they have been set.

  \sa setFilter() setSort()

*/

void QDataBrowser::refresh()
{
    d->cur.refresh();
}


/*! Performs an insert action on the browser's cursor. If there is no
   default cursor or no default form, nothing happens.

  If auto-editing is on (see setAutoEdit()), the following happens:

  <ul>
  <li> If the browser is already actively inserting a record,
  the current form's data is inserted into the database.
  <li> If the browser is not inserting a record, but the current record
  was changed by the user, the record is updated in the database with
  the current form's data (i.e. with the changes).
  </ul>

  If there is an error handling any of the above auto-edit actions,
  handleError() is called and no insert or update is performed.

  If no error occurred, or auto-editing is not enabled, the browser
  begins actively inserting a record into the database by performing
  the following actions:

  <ul>
  <li> The default cursor is primed for insert using QSqlCursor::primeInsert()
  <li> The primeInsert() signal is emitted
  <li> The form is updated with the values of the default cursor's
  edit buffer so that the user can fill in the values to be inserted
  </ul>

*/

void QDataBrowser::insert()
{
    QSqlRecord* buf = d->frm.record();
    QSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return;
    bool doIns = TRUE;
    QSql::Confirm conf = QSql::Yes;
    switch ( d->dat.mode() ) {
    case QSql::Insert:
	if ( autoEdit() ) {
	    if ( confirmInsert() )
		conf = confirmEdit( QSql::Insert );
	    switch ( conf ) {
	    case QSql::Yes:
		insertCurrent();
		break;
	    case QSql::No:
		break;
	    case QSql::Cancel:
		doIns = FALSE;
		break;
	    }
	}
	break;
    default:
	if ( autoEdit() && currentEdited() ) {
	    if ( confirmUpdate() )
		conf = confirmEdit( QSql::Update );
	    switch ( conf ) {
	    case QSql::Yes:
		updateCurrent();
		break;
	    case QSql::No:
		break;
	    case QSql::Cancel:
		doIns = FALSE;
		break;
	    }
	}
	break;
    }
    if ( doIns ) {
	d->dat.setMode( QSql::Insert );
	sqlCursor()->primeInsert();
	emit primeInsert( d->frm.record() );
	readFields();
    }
}


/*! Performs an update action on the browser's cursor.

  If there is no default cursor or no default form, nothing happens.
  Otherwise, the following happens:

  If the browser is actively inserting a record (see insert()), that
  record is inserted into the database using insertCurrent().
  Otherwise, the database is updated with the current form's data. If
  there is an error handling either action, handleError() is called.

*/

void QDataBrowser::update()
{
    QSqlRecord* buf = d->frm.record();
    QSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return;
    QSql::Confirm conf = QSql::Yes;
    switch ( d->dat.mode() ){
    case QSql::Insert:
	if ( autoEdit() ) {
	    if ( confirmInsert() )
		conf = confirmEdit( QSql::Insert );
	    switch ( conf ) {
	    case QSql::Yes:
		if ( insertCurrent() )
		    d->dat.setMode( QSql::Update );
		break;
	    case QSql::No:
		d->dat.setMode( QSql::Update );
		cur->editBuffer( TRUE );
		readFields();
		break;
	    case QSql::Cancel:
		break;
	    }
	} else
	    readFields();
	break;
    default:
	d->dat.setMode( QSql::Update );
	if ( confirmUpdate() )
	    conf = confirmEdit( QSql::Update );
	switch ( conf ) {
	case QSql::Yes:
	    updateCurrent();
	    break;
	case QSql::No:
	case QSql::Cancel:
	    break;
	}
	break;
    }
}


/*! Performs a delete action on the browser's cursor. If there is no
   default cursor or no default form, nothing happens.

  Otherwise, the following happens:

  If the browser is actively inserting a record (see insert()), the
  insert action is cancelled, and the browser returns to the last
  valid record.  Otherwise, the current form record is deleted from
  the database. If there is an error, handleError() is called.

*/

void QDataBrowser::del()
{
    QSqlRecord* buf = d->frm.record();
    QSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return;
    QSql::Confirm conf = QSql::Yes;
    switch ( d->dat.mode() ){
    case QSql::Insert:
	if ( confirmCancels() )
	    conf = confirmCancel( QSql::Insert );
	if ( conf == QSql::Yes ) {
	    cur->editBuffer( TRUE ); /* restore from cursor */
	    readFields();
	    d->dat.setMode( QSql::Update );
	} else
	    d->dat.setMode( QSql::Insert );
	break;
    default:
	if ( confirmDelete() )
	    conf = confirmEdit( QSql::Delete );
	switch ( conf ) {
	case QSql::Yes:
	    deleteCurrent();
	    break;
	case QSql::No:
	case QSql::Cancel:
	    break;
	}
	d->dat.setMode( QSql::Update );
	break;
    }
}


/*!  Moves the default cursor to the first record and refreshes the
  default form. If there is no default form or no default cursor,
  nothing happens.  If the browser successfully moved to the first
  record, the default cursor is primed for update and the
  primeUpdate() signal is emitted.

  If the browser is already positioned on the first record nothing
  happens.

*/

void QDataBrowser::first()
{
    nav( First );
}


/*!  Moves the default cursor to the last record and refreshes the
  default form. If there is no default form or no default cursor,
  nothing happens.  If the browser successfully moved to the last
  record, the default cursor is primed for update and the
  primeUpdate() signal is emitted.

  If the browser is already positioned on the last record nothing
  happens.
*/

void QDataBrowser::last()
{
    nav( Last );
}


/*!  Moves the default cursor to the next record and refreshes the
  default form. If there is no default form or no default cursor,
  nothing happens.  If the browser successfully moved to the next
  record, the default cursor is primed for update and the
  primeUpdate() signal is emitted.

  If the browser is positioned on the last record nothing happens.
*/

void QDataBrowser::next()
{
    nav( Next );
}


/*!  Moves the default cursor to the previous record and refreshes the
  default form. If there is no default form or no default cursor,
  nothing happens.  If the browser successfully moved to the previous
  record, the default cursor is primed for update and the
  primeUpdate() signal is emitted.

  If the browser is positioned on the first record nothing happens.
*/

void QDataBrowser::prev()
{
    nav( Prev );
}

/*! Reads the fields from the default cursor's edit buffer and displays
  them in the form.  If there is no default cursor or no default form,
  nothing happens.

*/

void QDataBrowser::readFields()
{
    d->frm.readFields();
}


/*!  Writes the form's data to the default cursor's edit buffer.  If
   there is no default cursor or no default form, nothing happens.

*/

void QDataBrowser::writeFields()
{
    d->frm.writeFields();
}


/*! Clears all the values in the form. For example the text of
   QLineEdit's will be set to "".

*/

void QDataBrowser::clearValues()
{
    d->frm.clearValues();
}

/*!  Reads the fields from the default form into the default cursor
  and performs an insert on the default cursor.  If there is no
  default form or default cursor, nothing happens.  If an error
  occurred during the insert into the database, handleError() is
  called and FALSE is returned. If the insert was successfull, the
  cursor is refreshed and relocated to the newly inserted record, the
  cursorChanged() signal is emitted, and TRUE is returned.

  \sa cursorChanged() sqlCursor() form() handleError()

*/

bool QDataBrowser::insertCurrent()
{
    if ( isReadOnly() )
	return FALSE;
    QSqlRecord* buf = d->frm.record();
    QSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return FALSE;
    writeFields();
    int ar = cur->insert();
    if ( !ar || !cur->isActive() ) {
	handleError( cur->lastError() );
	refresh();
	updateBoundary();
    } else {
	refresh();
	d->cur.findBuffer( cur->primaryIndex() );
	updateBoundary();
	cursorChanged( QSqlCursor::Insert );
	return TRUE;
    }
    return FALSE;
}


/*!  Reads the fields from the default form into the default cursor
  and performs an update on the default cursor. If there is no default
  form or default cursor, nothing happens.  If an error occurred during
  the update on the database, handleError() is called and FALSE is
  returned.  If the update was successfull, the cursor is refreshed and
  relocated to the updated record, the cursorChanged() signal is
  emitted, and TRUE is returned.

  \sa cursor() form() handleError()

*/

bool QDataBrowser::updateCurrent()
{
    if ( isReadOnly() )
	return FALSE;
    QSqlRecord* buf = d->frm.record();
    QSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return FALSE;
    writeFields();
    int ar = cur->update();
    if ( !ar || !cur->isActive() ) {
	handleError( cur->lastError() );
	refresh();
	updateBoundary();
    } else {
	refresh();
	d->cur.findBuffer( cur->primaryIndex() );
	updateBoundary();
	cur->editBuffer( TRUE );
	cursorChanged( QSqlCursor::Update );
	readFields();
	return TRUE;
    }
    return FALSE;
}


/*!  Performs a delete on the default cursor using the values from the
  default form and updates the default form. If there is no default
  form or default cursor, nothing happens.  If the delete was
  successful, the cursor is repositioned to the nearest record and TRUE
  is returned. The nearest record is the next record if there is one
  otherwise the previous record if there is one. If an error occurred
  during the delete from the database, handleError() is called and FALSE
  is returned.

  \sa cursor() form() handleError()

*/

bool QDataBrowser::deleteCurrent()
{
    if ( isReadOnly() )
	return FALSE;
    QSqlRecord* buf = d->frm.record();
    QSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return FALSE;
    writeFields();
    int n = cur->at();
    int ar = cur->del();
    if ( ar ) {
	refresh();
	updateBoundary();
	cursorChanged( QSqlCursor::Delete );
	if ( !cur->seek( n ) )
	    last();
	cur->editBuffer( TRUE );
	readFields();
	return TRUE;
    } else {
	if ( !cur->isActive() ) {
	    handleError( cur->lastError() );
	    refresh();
	    updateBoundary();
	}
    }
    return FALSE;
}


/*! Returns TRUE if the form's edit buffer differs from the current
  cursor buffer, otherwise FALSE is returned.

*/

bool QDataBrowser::currentEdited()
{
    QSqlRecord* buf = d->frm.record();
    QSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return FALSE;
    if ( !cur->isActive() || !cur->isValid() )
	return FALSE;
    writeFields();
    for ( uint i = 0; i < cur->count(); ++i ) {
	if ( cur->value(i) != buf->value(i) )
	    return TRUE;
    }
    return FALSE;
}

/*! \internal

  Navigate default cursor according to \a nav.  Handles autoEdit.

*/
void QDataBrowser::nav( Nav nav )
{
    QSqlRecord* buf = d->frm.record();
    QSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return;

    if ( !isReadOnly() && autoEdit() && currentEdited() ) {
	bool ok = TRUE;
	QSql::Confirm conf = QSql::Yes;
	switch ( d->dat.mode() ){
	case QSql::Insert:
	    if ( confirmInsert() )
		conf = confirmEdit( QSql::Insert );
	    switch ( conf ) {
	    case QSql::Yes:
		ok = insertCurrent();
		d->dat.setMode( QSql::Update );
		break;
	    case QSql::No:
		d->dat.setMode( QSql::Update );
		break;
	    case QSql::Cancel:
		return;
		break;
	    }
	    break;
	default:
	    if ( confirmUpdate() )
		conf = confirmEdit( QSql::Update );
	    switch ( conf ) {
	    case QSql::Yes:
		ok = updateCurrent();
		break;
	    case QSql::No:
		break;
	    case QSql::Cancel:
		return;
		break;
	    }
	}
	if ( !ok )
	    return;
    }
    int b = FALSE;
    switch( nav ) {
    case First:
	b = cur->first();
	break;
    case Last:
	b = cur->last();
	break;
    case Next:
	b = cur->next();
	break;
    case Prev:
	b = cur->prev();
	break;
    }
    if ( b ) {
	currentChanged( cur );
	cur->primeUpdate();
	emit primeUpdate( buf );
	readFields();
    }
    updateBoundary();
}

/*! \internal

  If boundaryChecking() is TRUE, checks the boundary of the current
  default cursor and emits signals which indicate the position of the
  cursor.
*/

void QDataBrowser::updateBoundary()
{
    if ( d->boundaryCheck ) {
	Boundary bound = boundary();
	switch ( bound ) {
	case Unknown:
	case None:
	    emit firstRecordAvailable( TRUE );
	    emit prevRecordAvailable( TRUE );
	    emit nextRecordAvailable( TRUE );
	    emit lastRecordAvailable( TRUE );
	    break;

	case BeforeBeginning:
	    emit firstRecordAvailable( FALSE );
	    emit prevRecordAvailable( FALSE );
	    emit nextRecordAvailable( TRUE );
	    emit lastRecordAvailable( TRUE );
	    break;

	case Beginning:
	    emit firstRecordAvailable( FALSE );
	    emit prevRecordAvailable( FALSE );
	    emit nextRecordAvailable( TRUE );
	    emit lastRecordAvailable( TRUE );
	    break;

	case End:
	    emit firstRecordAvailable( TRUE );
	    emit prevRecordAvailable( TRUE );
	    emit nextRecordAvailable( FALSE );
	    emit lastRecordAvailable( FALSE );
	    break;

	case AfterEnd:
	    emit firstRecordAvailable( TRUE );
	    emit prevRecordAvailable( TRUE );
	    emit nextRecordAvailable( FALSE );
	    emit lastRecordAvailable( FALSE );
	    break;
	}
    }
}

void QDataBrowser::handleError( const QSqlError& error )
{
    d->dat.handleError( this, error );
}

/*!  Protected virtual function which returns a confirmation for an
  edit of mode \a m.  Derived classes can reimplement this function
  and provide their own confirmation dialog.  The default
  implementation uses a message box which prompts the user to confirm
  the edit action.

*/

QSql::Confirm QDataBrowser::confirmEdit( QSql::Op m )
{
    return d->dat.confirmEdit( this, m );
}

/*!  Protected virtual function which returns a confirmation for
   cancelling an edit mode \a m.  Derived classes can reimplement this
   function and provide their own confirmation dialog.  The default
   implementation uses a message box which prompts the user to confirm
   the edit action.

*/

QSql::Confirm  QDataBrowser::confirmCancel( QSql::Op m )
{
    return d->dat.confirmCancel( this, m );
}


#endif
