/****************************************************************************
**
** Implementation of sql manager classes
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

#include "qsqlmanager_p.h"

#ifndef QT_NO_SQL

#include "qapplication.h"
#include "qsqlcursor.h"
#include "qsqlform.h"
#include "qsqldriver.h"

class QSqlCursorManager::QSqlCursorManagerPrivate
{
public:
    QSqlCursorManagerPrivate() : cur( 0 ), autoDelete( FALSE ) {}
    QString ftr;
    QStringList srt;
    QSqlCursor* cur;
    bool autoDelete;
};

/*!
  \class QSqlCursorManager qsqlmanager_p.h
  \brief The QSqlCursorManager class manages a database cursor.

  \module sql

  This class provides common cursor management functionality.  This
  includes saving and applying sorts and filters, refreshing (i.e.,
  re-selecting) the cursor and searching for records within the
  cursor.

*/

/*!  Constructs a cursor manager.

*/

QSqlCursorManager::QSqlCursorManager()
{
    d = new QSqlCursorManagerPrivate();
}


/*! Destroys the object and frees any allocated resources.

*/

QSqlCursorManager::~QSqlCursorManager()
{
    if ( d->autoDelete )
	delete d->cur;
    delete d;
}

/*! Sets the manager's sort to the index \a sort.  To apply the new
  sort, use refresh().

 */

void QSqlCursorManager::setSort( const QSqlIndex& sort )
{
    setSort( sort.toStringList() );
}

/*! Sets the manager's sort to the stringlist \a sort.  To apply the
  new sort, use refresh().

 */

void QSqlCursorManager::setSort( const QStringList& sort )
{
    d->srt = sort;
}

/*! Returns the current sort, or an empty stringlist if there is none.

*/

QStringList  QSqlCursorManager::sort() const
{
    return d->srt;
}

/*! Sets the manager's filter to the string \a filter.  To apply the
  new filter, use refresh().

*/

void QSqlCursorManager::setFilter( const QString& filter )
{
    d->ftr = filter;
}

/*! Returns the current filter, or an empty string if there is none.

*/

QString QSqlCursorManager::filter() const
{
    return d->ftr;
}

/*! Sets the default cursor used by the manager to \a cursor.  If \a
  autoDelete is TRUE (the default is FALSE), the manager takes
  ownership of the \a cursor pointer, which will be deleted when the
  manager is destroyed, or when setCursor() is called again. To
  activate the \a cursor use refresh().

  \sa cursor()

*/

void QSqlCursorManager::setCursor( QSqlCursor* cursor, bool autoDelete )
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

QSqlCursor* QSqlCursorManager::cursor() const
{
    return d->cur;
}


/*! Refreshes the manager using the default cursor.  The manager's
  filter and sort are applied.

  \sa setFilter() setSort()

*/

void QSqlCursorManager::refresh()
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
    QSqlCursor* myCursor = myManager.cursor();
    ...
    QSqlRecord* buf = myCursor->primeUpdate();
    buf->setValue( "name", "Dave" );
    buf->setValue( "city", "Oslo" );
    ...
    myCursor->update();  // update current record
    myCursor->select();  // refresh the cursor
    myManager.findBuffer( myCursor->primaryIndex() ); // go to the updated record
\endcode

*/

//## possibly add sizeHint parameter
bool QSqlCursorManager::findBuffer( const QSqlIndex& idx, int atHint )
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return FALSE;
    if ( !cur->isActive() )
	return FALSE;
    if ( !idx.count() )
	return FALSE;

    QSqlRecord* buf = cur->editBuffer();

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
	/* binary-like search based on record buffer and current sort fields */
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
	QApplication::setOverrideCursor( Qt::waitCursor );
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
	QApplication::restoreOverrideCursor();
    }

    return indexEquals;
}


class QSqlFormManager::QSqlFormManagerPrivate
{
public:
    QSqlFormManagerPrivate() : frm(0), rcd(0) {}
    QSqlForm* frm;
    QSqlRecord* rcd;
};


/*! Creates a form manager.

*/

QSqlFormManager::QSqlFormManager()
{
    d = new QSqlFormManagerPrivate();
}

/*! Destroys the object and frees any allocated resources.

*/

QSqlFormManager::~QSqlFormManager()
{
    delete d;
}

/*!  Clears the default form values.  If there is no default form,
  nothing happens,

*/

void QSqlFormManager::clearValues()
{
    if ( form() )
	form()->clearValues();
}

/*! Sets the form used by the form manager to \a form.  If a record has
  already been assigned to the form manager, that record is also used by
  the \a form to display data.

  \sa form()

*/

void QSqlFormManager::setForm( QSqlForm* form )
{
    d->frm = form;
    if ( d->rcd )
	d->frm->setRecord( d->rcd );
}


/*! Returns the default form used by the form manager, or 0 if there is
  none.

  \sa setForm()

*/

QSqlForm* QSqlFormManager::form()
{
    return d->frm;
}


/*! Sets the record used by the form manager to \a record.  If a form has
  already been assigned to the form manager, \a record is also used by
  the default form to display data.

  \sa record()

*/

void QSqlFormManager::setRecord( QSqlRecord* record )
{
    d->rcd = record;
    if ( d->frm )
	d->frm->setRecord( d->rcd );
}


/*! Returns the default record used by the form manager, or 0 if there is
  none.

  \sa setRecord()
*/

QSqlRecord* QSqlFormManager::record()
{
    return d->rcd;
}


/*! Causes the default form to read its fields .  If there is no
  default form, nothing happens.

  \sa setForm()

*/

void QSqlFormManager::readFields()
{
    if ( d->frm )
	d->frm->readFields();
}

/*! Causes the default form to write its fields .  If there is no
  default form, nothing happens.

  \sa setForm()

*/

void QSqlFormManager::writeFields()
{
    if ( d->frm )
	d->frm->writeFields();
}

#endif
