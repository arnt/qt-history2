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

  \brief The QDataBrowser class provides data manipulation and
  navigation for data entry forms.

  \module sql

    This class is used to manipulate and navigate data entry forms.  A
    high-level API is provided to navigate through data records in a
    cursor, insert, update and delete records, and refresh data in the
    display.

    A QDataBrowser is used to associate a dataset with a form in much
    the same way as a QDataTable associates a dataset with a table. Once
    the data browser has been constructed it can be associated with a
    dataset with setCursor() (or setSqlCursor()), and with a form with
    setForm(). Boundary checking, sorting and filtering can be set with
    setBoundaryChecking(), setSort() and setFilter(), respectively. 

    The insertCurrent() function reads the fields from the default form
    into the default cursor and performs the insert. The updateCurrent()
    and deleteCurrent() functions perform similarly to update and delete
    the current record respectively. 
    
    The user can be asked to confirm all edits with setConfirmEdits().
    For more precise control use setConfirmInsert(), setConfirmUpdate(),
    setConfirmDelete() and setConfirmCancels(). Use setAutoEdit() to
    control the behaviour of the form when the user edits a record and
    then navigates. 

    The record set is navigated using first(), next(), prev(), last()
    and seek(). The form's display is updated with refresh(). When
    navigation takes place the firstRecordAvailable(),
    lastRecordAvailable(), nextRecordAvailable() and
    prevRecordAvailable() signals are emitted. When the cursor record is
    changed due to navigation the cursorChanged() signal is emitted. 

    If you want finer control of the insert, update and delete processes
    then you can use the low level functions to perform these
    operations as described below.

    The form is populated with data from the database with readFields().
    If the user is allowed to edit, (see setReadOnly()), write the
    form's data back to the cursor's edit buffer with writeFields(). You
    can clear the values in the form with clearValues(). Editing is
    performed as follows:
    <ul>
    <li>\e insert When the data browser enters insertion mode it emits the
    primeInsert() signal which you can connect to, for example to
    pre-populate fields. Call writeFields() to write the user's edits to
    the cursor's edit buffer then call insert() to insert the record
    into the database. The beforeInsert() signal is emitted just before
    the cursor's edit buffer is inserted into the database; connect to
    this for example, to populate fields such as an auto-generated
    primary key. 
    <li>\e update For updates the primeUpdate() signal is emitted when
    the data browser enters update mode. After calling writeFields()
    call update() to update the record and connect to the beforeUpdate()
    signal to manipulate the user's data before the update takes place. 
    <li>\e delete For deletion the primeDelete() signal is emitted when
    the data browser enters deletion mode. After calling writeFields()
    call del() to delete the record and connect to the beforeDelete()
    signal, for example to record an audit of the deleted record. 
    </ul>

*/

/*! \enum QDataBrowser::Boundary

  This enum describes where the data browser is positioned.

  The currently defined values are:

  \value Unknown  the boundary cannot be determined (usually because
  there is no default cursor, or the default cursor is not active).

  \value None  the browser is not positioned on a boundary.

  \value BeforeBeginning  the browser is positioned before the
  beginning of the available records.

  \value Beginning  the browser is positioned at the beginning of
  the available records.

  \value End  the browser is positioned at the end of
  the available records.

  \value AfterEnd  the browser is positioned after the end of the
  available records.
*/

/*! Constructs a data browser which is a child of \a parent, with the
  name \a name and widget flags set to \a fl.

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


/*! \property QDataBrowser::boundaryChecking

    \brief whether boundary checking is active

    When boundary checking is active, signals are emitted indicating
    the current position of the default cursor.

  \sa boundary()
*/

void QDataBrowser::setBoundaryChecking( bool active )
{
    d->boundaryCheck = active;
}

bool QDataBrowser::boundaryChecking() const
{
    return d->boundaryCheck;
}

/*! \property QDataBrowser::sort

  \brief the browser sort

  The browser sort affects the order in which data records are viewed
  in the browser.  To actually apply the new sort, use refresh().

  When examining the sort property, a stringlist is returned in the
  form 'fieldname order', e.g.  'id ASC', 'surname DESC'.

  \sa filter() refresh()

*/

void QDataBrowser::setSort( const QStringList& sort )
{
    d->cur.setSort( sort );
}

/*! \overload

  Sets the browser's sort to the string list \a sort.  To apply the new
  sort, use refresh().

*/
void QDataBrowser::setSort( const QSqlIndex& sort )
{
    d->cur.setSort( sort );
}

QStringList QDataBrowser::sort() const
{
    return d->cur.sort();
}


/*! \property QDataBrowser::filter

  \brief the data filter for the browser

  The filter applies to the data shown in the browser. To actually
  apply the new filter, use refresh(). A filter string is an SQL WHERE
  clause without the WHERE keyword.

  \sa sort()

*/

void QDataBrowser::setFilter( const QString& filter )
{
    d->cur.setFilter( filter );
}


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
    if ( !cursor )
	return;
    d->cur.setCursor( cursor, autoDelete );
    d->frm.setRecord( cursor->editBuffer() );
    if ( cursor->isReadOnly() )
	setReadOnly( TRUE );
}


/*! Sets the default cursor used by the browser to \a cursor. If \a
   autoDelete is TRUE, the browser will take ownership of the \a
   cursor and delete it when appropriate.  This function is a wrapper
   for setCursor() and is provided purely for consistency with
   sqlCursor(). We recommend using setCursor() directly.

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

/*!  \property QDataBrowser::readOnly

  \brief whether the browser is read-only

  If the browse is read-only, no database edits will be allowed.

*/

void QDataBrowser::setReadOnly( bool active )
{
    d->readOnly = active;
}

bool QDataBrowser::isReadOnly() const
{
    return d->readOnly;
}

void QDataBrowser::setConfirmEdits( bool confirm )
{
    d->dat.setConfirmEdits( confirm );
}

/*! \property QDataBrowser::confirmInsert

  \brief whether the browser confirms insert operations

  If the confirmInsert property is active, the browser confirms all
  insert operations, otherwise all insert operations happen immediately.

  \sa confirmCancels() confirmEdits() confirmUpdate() confirmDelete()
*/

void QDataBrowser::setConfirmInsert( bool confirm )
{
    d->dat.setConfirmInsert( confirm );
}

/*! \property QDataBrowser::confirmUpdate

  \brief whether the browser confirms update operations

  If the confirmUpdate property is active, the browser confirms all
  update operations, otherwise all update operations happen immediately.

  \sa confirmCancels() confirmEdits() confirmInsert() confirmDelete()
*/

void QDataBrowser::setConfirmUpdate( bool confirm )
{
    d->dat.setConfirmUpdate( confirm );
}

/*! \property QDataBrowser::confirmDelete

  \brief whether the browser confirms delete operations

  If the confirmDelete property is active, the browser confirms all
  delete operations, otherwise all delete operations happen immediately.

  \sa confirmCancels() confirmEdits() confirmUpdate() confirmInsert()
*/

void QDataBrowser::setConfirmDelete( bool confirm )
{
    d->dat.setConfirmDelete( confirm );
}

/*! \property QDataBrowser::confirmEdits

  \brief whether the browser confirms edit operations

  If the confirmEdits property is active, the browser confirms all
  edit operations (inserts, updates and deletes) with the user (this
  behavior can be changed by reimplementing the confirmEdit() function),
  otherwise all edit operations happen immediately.

  \sa confirmEdit() confirmCancels() confirmInsert() confirmUpdate() confirmDelete()
*/

bool QDataBrowser::confirmEdits() const
{
    return ( d->dat.confirmEdits() );
}

bool QDataBrowser::confirmInsert() const
{
    return ( d->dat.confirmInsert() );
}

bool QDataBrowser::confirmUpdate() const
{
    return ( d->dat.confirmUpdate() );
}

bool QDataBrowser::confirmDelete() const
{
    return ( d->dat.confirmDelete() );
}

/*! \property QDataBrowser::confirmCancels

  \brief whether the browser confirms cancel operations

  If the confirmCancel property is active, all cancels must be
  confirmed by the user through a message box (this behavior can be
  changed by overriding the confirmCancel() function), otherwise all
  cancels occur immediately.

  \sa confirmEdits() confirmCancel()
*/

void QDataBrowser::setConfirmCancels( bool confirm )
{
    d->dat.setConfirmCancels( confirm );
}

bool QDataBrowser::confirmCancels() const
{
    return d->dat.confirmCancels();
}

/*! \property QDataBrowser::autoEdit

  \brief whether the browser automatically applies edits

  The default value for this property is TRUE. When the user begins an
  insert or update on a form there are two possible outcomes when they
  navigate to another record:

  \list 1
  \i the insert or update is is performed -- this occurs if autoEdit is TRUE
  \i the insert or update is abandoned -- this occurs if autoEdit is FALSE
  \endlist
*/

void QDataBrowser::setAutoEdit( bool autoEdit )
{
    d->dat.setAutoEdit( autoEdit );
}

bool QDataBrowser::autoEdit() const
{
    return d->dat.autoEdit();
}

/*! \fn void QDataBrowser::firstRecordAvailable( bool available )

  This signal is emitted whenever the position of the cursor changes.
  The \a available parameter indicates whether or not the first record
  in the default cursor is available.

*/

/*! \fn void QDataBrowser::lastRecordAvailable( bool available )

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

/*!  Moves the default cursor to the record specified by the index \a
  i and refreshes the default form. If there is no default form or no
  default cursor, nothing happens.  If \a relative is TRUE (the
  default is FALSE), the cursor is moved relative to its current
  position.  If the browser successfully moved to the desired record,
  the default cursor is primed for update and the primeUpdate() signal
  is emitted.

  If the browser is already positioned on the desired record nothing
  happens.

*/

bool QDataBrowser::seek( int i, bool relative )
{
    int b = 0;
    QSqlCursor* cur = d->cur.cursor();
    if ( !cur )
	return FALSE;
    if ( preNav() )
	b = cur->seek( i, relative );
    postNav( b );
    return b;
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
    nav( &QSqlCursor::first );
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
    nav( &QSqlCursor::last );
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
    nav( &QSqlCursor::next );
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
    nav( &QSqlCursor::prev );
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


/*! Clears all the values in the form.

    All the edit buffer field values are set to their 'zero state', e.g.
    0 for numeric fields, "" for string fields. Then the widgets are
    updated using the property map. A combobox that is property-mapped
    to ints would scroll to the first item for example. See the \l
    QSqlPropertyMap constructor for the default mappings of widgets to
    properties.

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
    emit beforeInsert( buf );
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
    emit beforeUpdate( buf );
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
    emit beforeDelete( buf );
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

  Pre-navigation checking.
*/

bool QDataBrowser::preNav()
{
    QSqlRecord* buf = d->frm.record();
    QSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return FALSE;

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
		return FALSE;
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
		return FALSE;
		break;
	    }
	}
	return ok;
    }
    return TRUE;
}

/*! \internal

  Handles post-navigation according to \a primeUpd.
*/

void QDataBrowser::postNav( bool primeUpd )
{
    if ( primeUpd ) {
	QSqlRecord* buf = d->frm.record();
	QSqlCursor* cur = d->cur.cursor();
	if ( !buf || !cur )
	    return;
	currentChanged( cur );
	cur->primeUpdate();
	emit primeUpdate( buf );
	readFields();
    }
    updateBoundary();
}

/*! \internal

  Navigate default cursor according to \a nav.  Handles autoEdit.

*/
void QDataBrowser::nav( Nav nav )
{
    int b = 0;
    QSqlCursor* cur = d->cur.cursor();
    if ( !cur )
	return;
    if ( preNav() )
	b = (cur->*nav)();
    postNav( b );
}

/*!  If boundaryChecking() is TRUE, checks the boundary of the current
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

/*! Virtual function which handles the error \a error.  The default
implementation warns the user with a message box.

*/

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

/*! \fn void QDataBrowser::beforeInsert( QSqlRecord* buf )
  This signal is emitted just before the cursor's edit buffer is inserted into the database.
  The \a buf parameter points to the edit buffer being inserted.
*/

/*! \fn void QDataBrowser::beforeUpdate( QSqlRecord* buf )
  This signal is emitted just before the cursor's edit buffer is updated in the database.
  The \a buf parameter points to the edit buffer being updated.
*/

/*! \fn void QDataBrowser::beforeDelete( QSqlRecord* buf )
  This signal is emitted just before the cursor's edit buffer  is deleted from the database.
  The \a buf parameter points to the edit buffer being deleted.
*/

#endif
