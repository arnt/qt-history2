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
    QDataBrowserPrivate() : boundaryCheck( TRUE ) {}
    QSqlCursorManager cur;
    QSqlFormManager frm;
    QDataManager dat;
    bool boundaryCheck;
};

/*!

  \class QDataBrowser qdatabrowser.h
  \brief SQL cursor/form manipulation and navigation

  \module sql

  This class is used to manipulate and navigate data entry forms.  A
  high-level API is provided to navigate through data records in a
  cursor, insert, update and delete records, and refresh data in the
  display.

  Convenient signals and slots are provided to navigate the cursor
  (see firstRecord(), lastRecord(), prevRecord(), nextRecord()), to
  edit records (see insertRecord(), updateRecord(), deleteRecord()),
  and to update the display according to the cursor's current position
  (see firstRecordAvailable(), lastRecordAvailable(),
  nextRecordAvailable(), prevRecordAvailable()).

*/

/*! \enum QDataBrowser::Boundary

  This enum type describes where the navigator is currently positioned.

  The currently defined values are:
  <ul>

  <li> \c Unknown - the boundary cannot be determined (usually because
  there is no default cursor, or the default cursor is not active).

  <li> \c None - the navigator is not positioned on a boundary.

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
  This is done by moving the default cursor and checking the position,
  however the current default form values will not be altered.  After
  checking for the boundary, the cursor is moved back to its former
  position.

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


/*! Sets boundary checking to \a active.  Without boundary checking,
  signals are not emitted indicating the current position of the
  default cursor.

  \sa boundaryChecking()
*/

void QDataBrowser::setBoundaryChecking( bool active )
{
    d->boundaryCheck = active;
}

/*! Returns TRUE if boundary checking is enabled (the default),
  otherwise FALSE is returned.

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
  new filter, use refresh().

*/

void QDataBrowser::setFilter( const QString& filter )
{
    d->cur.setFilter( filter );
}


/*! Returns the current filter, or an empty string if there is none.

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
  used in the default form to edit/view records.

  \sa cursor() form() setForm()

*/

void QDataBrowser::setCursor( QSqlCursor* cursor, bool autoDelete )
{
    d->cur.setCursor( cursor, autoDelete );
    d->frm.setRecord( cursor->editBuffer() );
}


/*! ### same as above, provided for consistency with sqlCursor()

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


/*!  ###

*/

void QDataBrowser::setForm( QSqlForm* form )
{
    d->frm.setForm( form );
}


/*! ###

*/

QSqlForm* QDataBrowser::form()
{
    return d->frm.form();
}


/*!  Sets the auto-edit property of the browser to \a auto. The
  default is TRUE. ### more info

*/

void QDataBrowser::setAutoEdit( bool autoEdit )
{
    d->dat.setAutoEdit( autoEdit );
}


/*! Returns TRUE if the auto-edit property is on, otherwise FALSE is
  returned.

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

  This signal is emitted whenever the current cursor has changed
  position.  The \a record parameter points to the contents of the
  current cursor.

*/


/*! \fn void QDataBrowser::primeInsert( QSqlRecord* buf )

  This signal is emitted before the edit buffer is inserted into the
  database by the browser.  The \a buf parameter points to the record
  buffer being inserted.  Note that QSqlCursor::primeInsert() is \em
  not called on the default cursor, as this would corrupt values in
  the form.  Connect to this signal in order to, for example, prime
  the record buffer with default data values.

  \sa insertCurrent()

*/


/*! \fn void QDataBrowser::primeUpdate( QSqlRecord* buf )

  This signal is emitted before the edit buffer is updated in the
  database by the browser.  The \a buf parameter points to the record
  buffer being updated.  Note that QSqlCursor::primeUpdate() is \em
  not called on the default cursor, as this would corrupt values in
  the form.  Connect to this signal in order to, for example, ###?

*/

/*! \fn void QDataBrowser::primeDelete( QSqlRecord* buf )

  This signal is emitted before the edit buffer is deleted from the
  database by the browser.  The \a buf parameter points to the record
  buffer being deleted.  Note that QSqlCursor::primeDelete() is \em
  not called on the default cursor, as this would corrupt values in
  the form.  Connect to this signal in order to, for example, ###?

*/


/*! \fn void QDataBrowser::cursorChanged( QSqlCursor::Mode mode )

  This signal is emitted whenever the cursor record was changed due to
  navigation.  The \a mode parameter is the edit that just took place.

*/


/*! Refreshes the browser using the default cursor.  The navigator's
  filter and sort are applied.

  \sa setFilter() setSort()

*/

void QDataBrowser::refresh()
{
    d->cur.refresh();
}


/*! Performs an insert action on the browser. If there is no default
  cursor or no default form, nothing happens.

  If auto-editing is on (see setAutoEdit()), the following happens:

  <ul>
  <li >If the browser is already actively inserting a record, the current
  form is inserted into the database
  <li> If the browser not inserting a record, but the current record was
  changed by the user, the record is first saved in the database
  </ul>

  If there is an error handling any of the above auto-edit actions,
  handleError() is called and no insert is performed.

  Otherwise, the following happens:

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
    switch ( d->dat.mode() ) {
    case QSql::Insert:
	if ( autoEdit() && !insertCurrent() )
	    doIns = FALSE;
	break;
    default:
	if ( autoEdit() && currentEdited() ) {
	    if ( !updateCurrent() )
		doIns = FALSE;
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


/*! Performs an update action on the browser. If there is no default
  cursor or no default form, nothing happens.

  Otherwise, the following happens:

  If the browser is actively inserting a record (see insert()), that
  record is inserted into the database using insertCurrent().
  Otherwise, the current form is updated in the database.  If there is
  an error handling either action, handleError() is called.

*/

void QDataBrowser::update()
{
    QSqlRecord* buf = d->frm.record();
    QSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return;
    switch ( d->dat.mode() ){
    case QSql::Insert:
	insertCurrent();
	break;
    default:
	updateCurrent();
	break;
    }
    d->dat.setMode( QSql::Update );
}


/*! Performs a delete action on the browser. If there is no default
  cursor or no default form, nothing happens.

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
    switch ( d->dat.mode() ){
    case QSql::Insert:
	cur->editBuffer( TRUE ); /* restore from cursor */
	readFields();
	break;
    default:
	deleteCurrent();
	break;
    }
    d->dat.setMode( QSql::Update );
}


/*!  Moves the default cursor to the first record and updates the
  default form. If there is no default form or no default cursor,
  nothing happens.  If the browser successfully moved to the first
  record, the default cursor is primed for update and the
  primeUpdate() signal is emitted.

*/

void QDataBrowser::first()
{
    QSqlRecord* buf = d->frm.record();
    QSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return;
    if ( cur->first() ) {
	currentChanged( cur );
	cur->primeUpdate();
	emit primeUpdate( buf );
	d->frm.readFields();
    }
    updateBoundary();
}


/*!  Moves the default cursor to the last record and updates the
  default form. If there is no default form or no default cursor,
  nothing happens.  If the browser successfully moved to the last
  record, the default cursor is primed for update and the
  primeUpdate() signal is emitted.

*/

void QDataBrowser::last()
{
    QSqlRecord* buf = d->frm.record();
    QSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return;
    if ( cur->last() ) {
	currentChanged( cur );
	cur->primeUpdate();
	emit primeUpdate( buf );
	d->frm.readFields();
    }
    updateBoundary();
}


/*!  Moves the default cursor to the next record and updates the
  default form. If there is no default form or no default cursor,
  nothing happens.  If the browser successfully moved to the next
  record, the default cursor is primed for update and the
  primeUpdate() signal is emitted.

*/

void QDataBrowser::next()
{
    QSqlRecord* buf = d->frm.record();
    QSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return;
    if ( cur->next() ) {
	currentChanged( cur );
	cur->primeUpdate();
	emit primeUpdate( buf );
	d->frm.readFields();
    }
    updateBoundary();
}


/*!  Moves the default cursor to the previous record and updates the
  default form. If there is no default form or no default cursor,
  nothing happens.  If the browser successfully moved to the previous
  record, the default cursor is primed for update and the
  primeUpdate() signal is emitted.

*/

void QDataBrowser::prev()
{
    QSqlRecord* buf = d->frm.record();
    QSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return;
    if ( cur->prev() ) {
	currentChanged( cur );
	cur->primeUpdate();
	emit primeUpdate( buf );
	d->frm.readFields();
    }
    updateBoundary();
}

/*! Reads the fields from the default cursor edit buffer and displays
  them in the form.  If there is no default cursor or no default form,
  nothing happens.

*/

void QDataBrowser::readFields()
{
    d->frm.readFields();
}


/*!  Writes the form to the default cursor edit buffer.  If there is
  no default cursor or no default form, nothing happens.

*/

void QDataBrowser::writeFields()
{
    d->frm.writeFields();
}


/*! ###

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
    QSqlRecord* buf = d->frm.record();
    QSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return FALSE;
    writeFields();
    int ar = cur->insert();
    if ( !ar || !cur->isActive() )
	handleError( cur->lastError() );
    else {
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
  form, nothing happens.  If an error occurred during the update on
  the database, handleError() is called and FALSE is returned.  If the
  update was successfull, the cursor is refreshed and relocated to the
  updated record, the cursorChanged() function is called, and TRUE is
  returned.

  \sa cursor() form() handleError()

*/

bool QDataBrowser::updateCurrent()
{
    QSqlRecord* buf = d->frm.record();
    QSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return FALSE;
    writeFields();
    int ar = cur->update();
    if ( !ar || !cur->isActive() )
	handleError( cur->lastError() );
    else {
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
  default form and updates the default form.  If the delete was
  successful, the cursor is repositioned to the next record and TRUE
  is returned. If an error occurred during the delete from the
  database, handleError() is called and FALSE is returned.

  \sa cursor() form() handleError()

*/

bool QDataBrowser::deleteCurrent()
{
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
	if ( !cur->isActive() )
	    handleError( cur->lastError() );
    }
    return FALSE;
}


/*! Returns TRUE if the current edit buffer differs from the current
  cursor buffer, otherwise FALSE is returned.

*/

bool QDataBrowser::currentEdited()
{
    //## to do
    return TRUE;
}


/*! If boundaryChecking() is TRUE, checks the boundary of the current
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
	    emit firstRecordAvailable( TRUE );
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
	    emit lastRecordAvailable( TRUE );
	    break;
	}
    }
}

void QDataBrowser::handleError( const QSqlError& error )
{
    d->dat.handleError( error );
}

#endif
