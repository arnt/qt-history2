/****************************************************************************
**
** Implementation of QSqlCursor class
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

#include "qsqlcursor.h"

#ifndef QT_NO_SQL

#include "qsqldriver.h"
#include "qsqlresult.h"
#include "qdatetime.h"
#include "qsqldatabase.h"
#include "qsql.h"
#include "qmap.h"

class QSqlCursorPrivate
{
public:

    QSqlCursorPrivate( const QString& name )
	: lastAt( QSql::BeforeFirst ), nm( name ), srt( name ), md( 0 )
    {}
    ~QSqlCursorPrivate(){}

    int               lastAt;
    QString           nm;         //name
    QSqlIndex         srt;        //sort
    QString           ftr;        //filter
    int               md;         //mode
    QSqlIndex         priIndx;    //primary index
    QSqlRecord        editBuffer;
    QSqlRecordInfo    infoBuffer;
};

QString qOrderByClause( const QSqlIndex & i, const QString& prefix = QString::null )
{
    QString str;
    int k = i.count();
    if( k == 0 ) return QString::null;
    str = " order by " + i.toString( prefix );
    return str;
}

/*! \class QSqlCursor qsqlcursor.h
    \ingroup database

    \brief The QSqlCursor class provides browsing and editing of SQL
    tables and views.

    \module sql

    A QSqlCursor is a database record (see \l QSqlRecord) that
    corresponds to a table or view within an SQL database (see \l
    QSqlDatabase).  There are two buffers in a cursor, one used for
    browsing and one used for editing records.  Each buffer contains a
    list of fields which correspond to the fields in the table or
    view.

    When positioned on a valid record, the browse buffer contains the
    values of the current record's fields from the database.  The edit
    buffer is separate, and is used for editing existing records and
    inserting new records.

    For browsing data, a cursor must first select() data from the
    database. After a successful select() the cursor is active
    (isActive() returns TRUE), but is initially not positioned on a
    valid record (isValid() returns FALSE).  To position the cursor on
    a valid record, use one of the navigation functions, next(),
    prev(), first(), last(), or seek().  Once positioned on a valid
    record, data can be retrieved from the browse buffer using
    value().  If a navigation function is not successful, it returns
    FALSE, the cursor will no longer be positioned on a valid record
    and the values returned by value() are undefined.

    For example:

    \quotefile sql/overview/retrieve2/main.cpp
    \skipto QSqlCursor
    \printline QSqlCursor
    \printuntil }

    In the above example, a cursor is created specifying a table or
    view name in the database. Then, select() is called, which can be
    optionally parameterised to filter and order the records
    retrieved. Each record in the cursor is retrieved using next().
    When next() returns FALSE, there are no more records to process,
    and the loop terminates.

    For editing records (rows of data), a cursor contains a separate
    edit buffer which is independent of the fields used when browsing.
    The functions insert(), update() and del() operate on the edit
    buffer.  This allows the cursor to be repositioned to other
    records while simultaneously maintaining a separate buffer for
    edits.  You can get a pointer to the edit buffer using
    editBuffer().  The primeInsert(), primeUpdate() and primeDelete()
    functions also return a pointer to the edit buffer and prepare it
    for insert, update and delete respectively.  Edit operations only
    affect a single row at a time.  Note that update() and del()
    require that the table or view contain a primaryIndex() to ensure
    that edit operations affect a unique record within the database.

    For example:

    \quotefile sql/overview/update/main.cpp
    \skipto prices
    \printline prices
    \printuntil update
    \printline

    To edit an existing database record, first move to the record you
    wish to update.  Call primeUpdate() to get the pointer to the
    cursor's edit buffer.  Then use this pointer to modify the values
    in the edit buffer.  Finally, call update() to save the changes to
    the database.  The values in the edit buffer will be used to
    locate the appropriate record when updating the database (see
    primaryIndex()).

    Similarly, when deleting an existing database record, first move
    the record you wish to delete.  Then, call primeDelete() to get
    the pointer to the edit buffer.  Finally, call del() to delete the
    record from the database.  Again, the values in the edit buffer
    will be used to locate and delete the appropriate record.

    To insert a new record, call primeInsert() to get the pointer to
    the edit buffer.  Use this pointer to populate the edit buffer
    with new values and then insert() the record into the database.

    After calling insert(), update() or del(), the cursor is no longer
    positioned on a valid record and can no longer be navigated
    (isValid() return FALSE).  The reason for this is that any changes
    made to the database will not be visible until select() is called
    to refresh the cursor. You can change this behavior by passing
    FALSE to insert(), update() or del() which will prevent the cursor
    from becoming invalid. These edits will then not be visible when
    navigating the cursor until select() is called.

    QSqlCursor contains virtual methods which allow editing behavior
    to be customized by subclasses.  This allows custom cursors to be
    created which encapsulate the editing behavior of a database table
    for an entire application.  For example, a cursor can be
    customized to always auto-number primary index fields, or provide
    fields with suitable default values, when inserting new records.

*/

/*! \enum QSqlCursor::Mode

  This enum type describes how QSqlCursor operates on records in the
  database.

  The currently defined values are:

  \value ReadOnly  the cursor can only SELECT records from the
  database.

  \value Insert  the cursor can INSERT records into the database.

  \value Update  the cursor can UPDATE records in the database.

  \value Delete  the cursor can DELETE records from the database.

  \value Writable  the cursor can INSERT, UPDATE and DELETE records
  in the database.
*/

/*! Constructs a cursor on database \a db using table or view \a name.

  If \a autopopulate is TRUE (the default), the \a name of the
  cursor must correspond to an existing table or view name in the
  database so that field information can be automatically created.
  If the table or view does not exist, the cursor will not be
  functional.

  The cursor is created with an initial mode of QSqlCursor::Writable
  (meaning that records can be inserted, updated or deleted using the
  cursor). If the cursor does not have a unique primary index, update
  and deletes cannot be performed.

  Note that \a autopopulate refers to populating the cursor with
  meta-data, e.g. the names of the table's fields, not with retrieving
  data. The select() function is used to populate the cursor with
  data.

  \sa setName() setMode()

*/

QSqlCursor::QSqlCursor( const QString & name, bool autopopulate, QSqlDatabase* db )
    : QSqlRecord(), QSqlQuery( QString::null, db )
{
    d = new QSqlCursorPrivate( name );
    setMode( Writable );
    if ( !d->nm.isNull() )
	setName( d->nm, autopopulate );
}

/*!
  Constructs a copy of \a other.

*/

QSqlCursor::QSqlCursor( const QSqlCursor & other )
    : QSqlRecord( other ), QSqlQuery( other )
{
    d = new QSqlCursorPrivate( other.d->nm );
    d->lastAt = other.d->lastAt;
    d->nm = other.d->nm;
    d->srt = other.d->srt;
    d->ftr = other.d->ftr;
    d->priIndx = other.d->priIndx;
    setMode( other.mode() );
}

/*! Destroys the object and frees any allocated resources.

*/

QSqlCursor::~QSqlCursor()
{
    delete d;
}

/*!
  Sets the cursor equal to \a other.

*/

QSqlCursor& QSqlCursor::operator=( const QSqlCursor& other )
{
    QSqlRecord::operator=( other );
    QSqlQuery::operator=( other );
    delete d;
    d = new QSqlCursorPrivate( other.d->nm );
    d->lastAt = other.d->lastAt;
    d->nm = other.d->nm;
    d->srt = other.d->srt;
    d->ftr = other.d->ftr;
    d->priIndx = other.d->priIndx;
    setMode( other.mode() );
    return *this;
}

/*!  Sets the current sort to \a sort.  Note that no new records are
  selected.  To select new records, use select(). The \a sort will apply
  to any subsequent select() calls that do not explicitly specify a
  sort.

*/

void QSqlCursor::setSort( const QSqlIndex& sort )
{
    d->srt = sort;
}

/*!  Returns the current sort, or an empty index if there is no
  current sort.

*/
QSqlIndex QSqlCursor::sort() const
{
    return d->srt;
}

/*! Sets the current filter to \a filter.  Note that no new records
  are selected.  To select new records, use select(). The \a filter
  will apply to any subsequent select() calls that do not explicitly
  specify a filter.

  The filter is an SQL \c WHERE clause without the keyword 'WHERE', e.g.
  <tt>name='Dave'</tt>.


*/
void QSqlCursor::setFilter( const QString& filter )
{
    d->ftr = filter;
}

/*!  Returns the current filter, or an empty string if there is no
  current filter.

*/
QString QSqlCursor::filter() const
{
    return d->ftr;
}

/*!  Sets the name of the cursor to \a name.  If \a autopopulate is
  TRUE (the default), the \a name must correspond to a valid table or
  view name in the database.  Also, note that all references to the
  cursor edit buffer become invalidated when fields are
  auto-populated.  See the QSqlCursor constructor documentation for
  more information.

*/
void QSqlCursor::setName( const QString& name, bool autopopulate )
{
    d->nm = name;
    if ( autopopulate ) {
	if ( driver() ) {
	    d->infoBuffer = driver()->recordInfo( name );
	    *this = d->infoBuffer.toRecord();
	    d->editBuffer = *this;
	    d->priIndx = driver()->primaryIndex( name );
	}
#ifdef QT_CHECK_RANGE
	if ( isEmpty() )
	    qWarning("QSqlCursor::setName: unable to build record, does '%s' exist?", name.latin1() );
#endif
    }
}

/*!  Returns the name of the cursor.

*/

QString QSqlCursor::name() const
{
    return d->nm;
}

/*! \reimp
*/

QString QSqlCursor::toString( const QString& prefix, const QString& sep ) const
{
    QString pflist;
    QString pfix =  prefix.isNull() ? QString::null : prefix + ".";
    bool comma = FALSE;

    for ( uint i = 0; i < count(); ++i ) {
	const QString fname = fieldName( i );
	if ( isGenerated( i ) ) {
	    if( comma )
		pflist += sep + " ";
	    pflist += pfix + fname;
	    comma = TRUE;
	}
    }
    return pflist;
}

/*!
  \internal

  Assigns the record \a list.

*/
QSqlRecord & QSqlCursor::operator=( const QSqlRecord & list )
{
    return QSqlRecord::operator=( list );
}

/*!  Append a copy of field \a fieldInfo to the end of the cursor.  Note
  that all references to the cursor edit buffer become invalidated.

*/

void QSqlCursor::append( const QSqlFieldInfo& fieldInfo )
{
    d->editBuffer.append( fieldInfo.toField() );
    d->infoBuffer.append( fieldInfo );
    QSqlRecord::append( fieldInfo.toField() );
}

/*!
    \overload

    Insert a copy of \a fieldInfo at position \a pos.  If a field already
  exists at \a pos, it is removed.  Note that all references to the
  cursor edit buffer become invalidated.
*/

void  QSqlCursor::insert( int pos, const QSqlFieldInfo& fieldInfo )
{
    d->editBuffer.insert( pos, fieldInfo.toField() );
    d->infoBuffer.insert( d->infoBuffer.at( pos ), fieldInfo );
    QSqlRecord::insert( pos, fieldInfo.toField() );
}

/*!  Removes the field at \a pos.  If \a pos does not exist, nothing
  happens.  Note that all references to the cursor edit buffer become
  invalidated.

*/

void QSqlCursor::remove( int pos )
{
    d->editBuffer.remove( pos );
    d->infoBuffer.erase( d->infoBuffer.at( pos ) );
    QSqlRecord::remove( pos );
}

/*! Sets the generated flag for the field \a name to \a generated.  If the
  field does not exist, nothing happens. Only fields that have \a
  generated set to TRUE are included in the SQL that is generated, e.g.
  by QSqlCursor.

  \sa isGenerated()
*/

void QSqlCursor::setGenerated( const QString& name, bool generated )
{
    int pos = position( name );
    if ( pos == -1 )
	return;
    QSqlRecord::setGenerated( name, generated );
    d->editBuffer.setGenerated( name, generated );
    d->infoBuffer[ pos ].setGenerated( generated );
}

/*!  \overload

  Sets the generated flag for the field \a i to \a generated.

  \sa isGenerated()
*/
void QSqlCursor::setGenerated( int i, bool generated )
{
    if ( i < 0 || i >= (int)d->infoBuffer.count() )
	return;
    QSqlRecord::setGenerated( i, generated );
    d->editBuffer.setGenerated( i, generated );
    d->infoBuffer[i].setGenerated( generated );
}

/*!  Returns the primary index associated with the cursor as defined
  in the database, or an empty index if there is no primary index.  If
  \a setFromCursor is TRUE (the default), the index fields are populated
  with the corresponding values in the cursor's current record.

*/

QSqlIndex QSqlCursor::primaryIndex( bool setFromCursor ) const
{
    if ( setFromCursor ) {
	for ( uint i = 0; i < d->priIndx.count(); ++i ) {
	    const QString fn = d->priIndx.fieldName( i );
	    if ( contains( fn ) )
		d->priIndx.setValue( i, value( fn ) );
	}
    }
    return d->priIndx;
}

/*!  Sets the primary index associated with the cursor to the index \a
  idx.  Note that this index must contain a field or set of fields
  which identify a unique record within the underlying database table
  or view so that update() and del() will execute as expected.

  \sa update() del()

*/

void QSqlCursor::setPrimaryIndex( const QSqlIndex& idx )
{
    d->priIndx = idx;
}


/*!  Returns an index composed of \a fieldNames, all in ASCending
  order.  Note that all field names must exist in the cursor,
  otherwise an empty index is returned.

  \sa QSqlIndex
*/

QSqlIndex QSqlCursor::index( const QStringList& fieldNames ) const
{
    QSqlIndex idx;
    for ( QStringList::ConstIterator it = fieldNames.begin(); it != fieldNames.end(); ++it ) {
	const QSqlField* f = field( (*it) );
	if ( !f ) { /* all fields must exist */
	    idx.clear();
	    break;
	}
	idx.append( *f );
    }
    return idx;
}

/*!  \overload

  Returns an index based on \a fieldName.
*/

QSqlIndex QSqlCursor::index( const QString& fieldName ) const
{
    QStringList fl( fieldName );
    return index( fl );
}

/*! \overload

  Returns an index based on \a fieldName.
*/

QSqlIndex QSqlCursor::index( const char* fieldName ) const
{
    return index( QStringList( QString( fieldName ) ) );
}

/*!  Selects all fields in the cursor from the database matching the
  filter criteria \a filter.  The data is returned in the order
  specified by the index \a sort. Returns TRUE if the data was
  successfully selected, otherwise FALSE is returned.

  The \a filter is a string containing an SQL \c WHERE clause but
  without the 'WHERE' keyword. The cursor is initially positioned at
  an invalid row after this function is called.  To move to a valid
  row, use seek(), first(), last(), prev() or next().

    Example:
  \code
  QSqlCursor cur( "Employee" ); // Use the Employee table or view
  cur.select( "deptno=10" ); // select all records in department 10
  while( cur.next() ) {
      ... // process data
  }
  ...
  // select records in other departments, ordered by department number
  cur.select( "deptno>10", cur.index( "deptno" ) );
  ...
  \endcode

  The filter will apply to any subsequent select() calls that do not
  explicitly specify another filter. Similarly the sort will apply to
  any subsequent select() calls that do not explicitly specify another
  sort.

  \code
  QSqlCursor cur( "Employee" );
  cur.select( "deptno=10" ); // select all records in department 10
  while( cur.next() ) {
      ... // process data
  }
  ...
  cur.select(); // re-selects all records in department 10
  ...
  \endcode

*/

bool QSqlCursor::select( const QString & filter, const QSqlIndex & sort )
{
    QString fieldList = toString( d->nm );
    if ( fieldList.isEmpty() )
	return FALSE;
    QString str= "select " + fieldList;
    str += " from " + d->nm;
    if ( !filter.isEmpty() ) {
	d->ftr = filter;
	str += " where " + filter;
    } else
	d->ftr = QString::null;
    if ( sort.count() > 0 )
	str += " order by " + sort.toString( d->nm );
    d->srt = sort;
    d->lastAt = QSql::BeforeFirst;
    return exec( str );
}

/*!  \overload

  Selects all fields in the cursor from the database.  The rows are
  returned in the order specified by the last call to setSort() or the
  last call to select() that specified a sort, whichever is the most
  recent.  If there is no current sort, the order in which the rows are
  returned is undefined.  The records are filtered according to the
  filter specified by the last call to setFilter() or the last call to
  select() that specified a filter, whichever is the most recent. If
  there is no current filter, all records are returned.  The cursor is
  initially positioned at an invalid row.  To move to a valid row, use
  seek(), first(), last(), prev() or next().

  \sa setSort() setFilter()
*/

bool QSqlCursor::select()
{
    return select( filter(), sort() );
}

/*!  \overload

  Selects all fields in the cursor from the database.  The data is
  returned in the order specified by the index \a sort. The records
  are filtered according to the filter specified by the last call to
  setFilter() or the last call to select() that specified a filter,
  whichever is the most recent.  The cursor is initially positioned at
  an invalid row.  To move to a valid row, use seek(), first(),
  last(), prev() or next().

*/

bool QSqlCursor::select( const QSqlIndex& sort )
{
    return select( QString::null, sort );
}

/*! \overload

  Selects all fields in the cursor matching the filter index \a
  filter.  The data is returned in the order specified by the index \a
  sort. The \a filter index works by constructing a WHERE clause using
  the names of the fields from the \a filter and their values from the
  current cursor record. The cursor is initially positioned at an
  invalid row.  To move to a valid row, use seek(), first(), last(),
  prev() or next().  This function is useful, for example, for
  retrieving data based upon a table's primary index:

  \code
  QSqlCursor cur( "Employee" );
  QSqlIndex pk = cur.primaryIndex();
  cur.setValue( "id", 10 );
  cur.select( pk, pk ); // generates "SELECT ... FROM Employee WHERE id=10 ORDER BY id"
  ...
  \endcode

  In this example the QSqlIndex, pk, is used for two different purposes.
  When used as the filter (first) argument, the field names it contains
  are used to construct the WHERE clause, each set to the current cursor
  value, <tt>WHERE id=10</tt>, in this case. When used as the sort
  (second) argument the field names it contains are used for the ORDER
  BY clause, <tt>ORDER BY id</tt> in this example.

*/
bool QSqlCursor::select( const QSqlIndex & filter, const QSqlIndex & sort )
{
    return select( toString( filter, this, d->nm, "=", "and" ), sort );
}

/*!  Sets the cursor mode to \a mode.  This value can be an OR'ed
  combination of QSqlCursor::Mode values. The default mode for a
  cursor is QSqlCursor::Writable.

  \code
  QSqlCursor cur( "Employee" );
  cur.setMode( QSqlCursor::Writable ); // allow insert/update/delete
  ...
  cur.setMode( QSqlCursor::Insert | QSqlCursor::Update ); // allow inserts and updates only
  ...
  cur.setMode( QSqlCursor::ReadOnly ); // no inserts/updates/deletes allowed

  \endcode
*/

void QSqlCursor::setMode( int mode )
{
    d->md = mode;
}

/*! Returns the current cursor mode.

   \sa setMode()
*/

int QSqlCursor::mode() const
{
    return d->md;
}

/*! Sets field \a name to \a calculated.  If the field \a name does
  not exist, nothing happens.  The value of a calculated field is set
  by the calculateField() virtual function which you must reimplement
  otherwise the field value will become an invalid
  QVariant. Calculated fields do not appear in generated SQL
  statements sent to the database.

  \sa calculateField() QSqlRecord::setGenerated()
*/

void QSqlCursor::setCalculated( const QString& name, bool calculated )
{
    int pos = position( name );
    if ( pos < 0 )
	return;
    d->infoBuffer[ pos ].setCalculated( calculated );
    if ( calculated )
	setGenerated( pos, FALSE );
}

/*! Returns TRUE if the field \a name is calculated, otherwise FALSE is
  returned. If the field \a name does not exist, FALSE is returned.

  \sa setCalculated()
*/

bool QSqlCursor::isCalculated( const QString& name ) const
{
    int pos = position( name );
    if ( pos < 0 )
	return FALSE;
    return d->infoBuffer[ pos ].isCalculated();
}

/*! Sets field \a name to \a trim.  If the field \a name does not
  exist, nothing happens.

  When a trimmed field of type string or cstring is read from the
  database any trailing (right-most) spaces are removed.

  See \link qvariant.html QVariant\endlink.

  \sa isTrimmed()
*/

void QSqlCursor::setTrimmed( const QString& name, bool trim )
{
    int pos = position( name );
    if ( pos < 0 )
	return;
    d->infoBuffer[ pos ].setTrim( trim );
}

/*! Returns TRUE if the field \a name is trimmed, otherwise FALSE is
  returned. If the field \a name does not exist, FALSE is returned.

  When a trimmed field of type string or cstring is read from the
  database any trailing (right-most) spaces are removed.

  \sa setTrimmed()
*/

bool QSqlCursor::isTrimmed( const QString& name ) const
{
    int pos = position( name );
    if ( pos < 0 )
	return FALSE;
    return d->infoBuffer[ pos ].isTrim();
}

/*! Returns TRUE if the cursor is read-only, FALSE otherwise.  The
  default is FALSE.  Read-only cursors cannot be edited using
  insert(), update() or del().

   \sa setMode()
*/

bool QSqlCursor::isReadOnly() const
{
    return d->md == 0;
}

/*! Returns TRUE if the cursor will perform inserts, FALSE otherwise.

   \sa setMode()
*/

bool QSqlCursor::canInsert() const
{
    return ( ( d->md & Insert ) == Insert ) ;
}


/*! Returns TRUE if the cursor will perform updates, FALSE otherwise.

   \sa setMode()
*/

bool QSqlCursor::canUpdate() const
{
    return ( ( d->md & Update ) == Update ) ;
}

/*! Returns TRUE if the cursor will perform deletes, FALSE otherwise.

   \sa setMode()
*/

bool QSqlCursor::canDelete() const
{
    return ( ( d->md & Update ) == Update ) ;
}

/*! \overload

  Returns a formatted string composed of the \a prefix (e.g. table or
    view name), ".", the \a field name, the \a fieldSep and the field
    value. If the \a prefix is empty then the string will begin with the
    \a field name. This function is useful for generating SQL statements.

*/

QString QSqlCursor::toString( const QString& prefix, QSqlField* field, const QString& fieldSep ) const
{
    QString f;
    if ( field && driver() ) {
	f = ( prefix.length() > 0 ? prefix + QString(".") : QString::null ) + field->name();
	f += " " + fieldSep + " ";
	if ( field->isNull() ) {
	    f += "NULL";
	} else {
	    f += driver()->formatValue( field );
	}
    }
    return f;
}

/*! Returns a formatted string composed of all the fields in \a rec.
    Each field is composed of the \a prefix (e.g. table or view name),
    ".", the field name, the \a fieldSep and the field value. If the
    \a prefix is empty then the field will begin with the field name.
    The fields are then joined together separated by \a sep.
    Fields where isGenerated() returns FALSE are not included.
    This function is useful for generating SQL statements.

*/

QString QSqlCursor::toString( QSqlRecord* rec, const QString& prefix, const QString& fieldSep,
			      const QString& sep ) const
{
    QString filter;
    bool separator = FALSE;
    for ( uint j = 0; j < count(); ++j ) {
	QSqlField* f = rec->field( j );
	if ( rec->isGenerated( j ) ) {
	    if ( separator )
		filter += sep + " " ;
	    filter += toString( prefix, f, fieldSep );
	    separator = TRUE;
	}
    }
    return filter;
}

/*! \overload

    Returns a formatted string composed of all the fields in the index
    \a i. Each field is composed of the \a prefix (e.g. table or view
    name), ".", the field name, the \a fieldSep and the field value.
    If the \a prefix is empty then the field will begin with the field
    name. The field values are taken from \a rec. The fields are then
    joined together separated by \a sep. Fields where isGenerated() returns
    FALSE are ignored.
    This function is useful for generating SQL statements.

*/

QString QSqlCursor::toString( const QSqlIndex& i, QSqlRecord* rec, const QString& prefix,
				const QString& fieldSep, const QString& sep ) const
{
    QString filter;
    bool separator = FALSE;
    for( uint j = 0; j < i.count(); ++j ){
	if ( rec->isGenerated( j ) ) {
	    if( separator ) {
		filter += " " + sep + " " ;
	    }
	    QString fn = i.fieldName( j );
	    QSqlField* f = rec->field( fn );
	    filter += toString( prefix, f, fieldSep );
	    separator = TRUE;
	}
    }
    return filter;
}

/*! Inserts the current contents of the cursor's edit record buffer
    into the database, if the cursor allows inserts.  Returns the
    number of rows affected by the insert.  For error information, use
    lastError().

    If \a invalidate is TRUE (the default), the cursor will no longer
    be positioned on a valid record and can no longer be navigated. A
    new select() call must be made before navigating to a valid
    record.

    \quotefile sql/overview/insert2/main.cpp
    \skipto prices
    \printline prices
    \printuntil insert

    In the above example, a cursor is created on the 'prices' table
   and a pointer to the insert buffer is aquired using
   primeInsert(). Each field's value is set to the desired value and
   then insert() is called to insert the data into the
   database. Remember: all edit operations (insert(), update() and
   delete()) operate on the contents of the cursor edit buffer and not
   on the contents of the cursor itself.

  \sa setMode() lastError()
*/

int QSqlCursor::insert( bool invalidate )
{
    if ( ( d->md & Insert ) != Insert || !driver() )
	return FALSE;
    int k = d->editBuffer.count();
    if ( k == 0 )
	return 0;

    QString fList;
    QString vList;
    bool comma = FALSE;

    for( int j = 0; j < k; ++j ) {
	QSqlField* f = d->editBuffer.field( j );
	if ( d->editBuffer.isGenerated( j ) ) {
	    if ( comma ) {
		fList += ",";
		vList += ",";
	    }
	    fList += f->name();
	    vList += driver()->formatValue( f );
	    comma = TRUE;
	}
    }

    if ( !comma ) {
	// no valid fields found
	return 0;
    }

    QString str = QString( "insert into %1 (%2) values (%3)" ).arg( name() ).arg( fList ).arg( vList );
    return apply( str, invalidate );
}

/*!  Returns a pointer to the current internal edit buffer.  If \a
  copy is TRUE (the default is FALSE), the current cursor field values
  are first copied into the edit buffer.  The edit buffer is valid as
  long as the cursor remains valid.  The cursor retains ownership of
  the returned pointer, so it must not be deleted or modified.

  \sa primeInsert(), primeUpdate() primeDelete()
*/

QSqlRecord* QSqlCursor::editBuffer( bool copy )
{
    if ( copy ) {
	for(uint i = 0; i < d->editBuffer.count(); i++) {
	    if ( QSqlRecord::isNull( i ) ) {
		d->editBuffer.setNull( i );
	    } else {
		d->editBuffer.setValue( i, value( i ) );
	    }
	}
    }
    return &d->editBuffer;
}

/*!  'Primes' the field values of the edit buffer for update and
  returns a pointer to the edit buffer.  The default implementation
  copies the field values from the current cursor record into the edit
  buffer (therefore, this function is equivalent to calling
  editBuffer( TRUE ) ). The cursor retains ownership of the returned
  pointer, so it must not be deleted or modified.

  \sa editBuffer() update()

*/

QSqlRecord* QSqlCursor::primeUpdate()
{
    return editBuffer( TRUE );
}

/*!  'Primes' the field values of the edit buffer for delete and
  returns a pointer to the edit buffer.  The default implementation
  copies the field values from the current cursor record into the edit
  buffer (therefore, this function is equivalent to calling
  editBuffer( TRUE ) ).  The cursor retains ownership of the returned
  pointer, so it must not be deleted or modified.

  \sa editBuffer() del()

*/

QSqlRecord* QSqlCursor::primeDelete()
{
    return editBuffer( TRUE );
}

/*!  'Primes' the field values of the edit buffer for insert and
  returns a pointer to the edit buffer.  The default implementation
  clears all field values in the edit buffer.  The cursor retains
  ownership of the returned pointer, so it must not be deleted or
  modified.

  \sa editBuffer() insert()

*/

QSqlRecord* QSqlCursor::primeInsert()
{
    d->editBuffer.clearValues();
    return &d->editBuffer;
}


/*!  Updates the database with the current contents of the edit
  buffer.  Returns the number of records which were updated, or 0 if
  there was an error.  For error information, use lastError().

  Only records which meet the filter criteria specified by the
  cursor's primary index are updated.  If the cursor does not contain
  a primary index, no update is performed and 0 is returned.

  If \a invalidate is TRUE (the default), the current cursor can no
  longer be navigated. A new select() call must be made before you can
  move to a valid record.  For example:

  \quotefile sql/overview/update/main.cpp
  \skipto prices
  \printline prices
  \printuntil update
  \printline

  In the above example, a cursor is created on the 'prices' table and
  is positioned on the record to be update updated. A pointer is then
  aquired to the cursor's edit buffer using primeUpdate(). A new value
  is calculated and placed into the edit buffer with the setValue()
  call. Finally, an update() call is made on the cursor which uses the
  tables's primary index to update the record in the database with the
  contents of the cursor's edit buffer.  Remember: all edit operations
  (insert(), update() and delete()) operate on the contents of the
  cursor edit buffer and not on the contents of the cursor itself.

  Note that if the primary index does not uniquely distinguish records
  the database may be changed into an inconsistent state.

  \sa setMode() lastError()
*/

int QSqlCursor::update( bool invalidate )
{
    if ( primaryIndex().isEmpty() )
	return 0;
    return update( toString( primaryIndex(), &d->editBuffer, d->nm, "=", "and" ), invalidate );
}

/*!  \overload

  Updates the database with the current contents of the cursor edit
  buffer using the specified \a filter.  Returns the number of records
  which were updated, or 0 if there was an error.  For error
  information, use lastError().

  Only records which meet the filter criteria are updated, otherwise
  all records in the table are updated.

  If \a invalidate is TRUE (the default), the cursor can no
  longer be navigated. A new select() call must be made before you can
  move to a valid record.

  \sa primeUpdate() setMode() lastError()
*/

int QSqlCursor::update( const QString & filter, bool invalidate )
{
    if ( ( d->md & Update ) != Update )
	return FALSE;
    int k = count();
    if( k == 0 ) return 0;
    QString str = "update " + name();
    str += " set " + toString( &d->editBuffer, QString::null, "=", "," );
    if ( filter.length() )
	str+= " where " + filter;
    return apply( str, invalidate );
}

/*!  Deletes a record from the database using the cursor's primary
  index and the contents of the cursor edit buffer. Returns the number
  of records which were deleted, or 0 if there was an error.  For
  error information, use lastError().

  Only records which meet the filter criteria specified by the
  cursor's primary index are deleted.  If the cursor does not contain
  a primary index, no delete is performed and 0 is returned. If \a
  invalidate is TRUE (the default), the current cursor can no longer
  be navigated. A new select() call must be made before you can move
  to a valid record.  For example:

    \quotefile sql/overview/del/main.cpp
    \skipto prices
    \printline prices
    \printuntil }

   In the above example, a cursor is created on the 'prices' table and
   positioned to the record to be deleted.  First primeDelete() is
   called to populate the edit buffer with the current cursor values,
   e.g. with an id of 999, and then del() is called to actually delete
   the record from the database.  Remember: all edit operations
   (insert(), update() and delete()) operate on the contents of the
   cursor edit buffer and not on the contents of the cursor itself.

  \sa primeDelete() setMode() lastError()
*/

int QSqlCursor::del( bool invalidate )
{
    if ( primaryIndex().isEmpty() )
	return 0;
    return del( toString( primaryIndex(), &d->editBuffer, d->nm,
			  "=", "and" ), invalidate );
}

/*! \overload

   Deletes the current cursor record from the database using the
   filter \a filter.  Only records which meet the filter criteria are
   deleted.  Returns the number of records which were deleted. If \a
   invalidate is TRUE (the default), the current cursor can no longer
   be navigated. A new select() call must be made before you can move to
   a valid record. For error information, use lastError().

   The \a filter is an SQL \c WHERE clause, e.g. <tt>id=500</tt>.

   \sa setMode() lastError()
*/

int QSqlCursor::del( const QString & filter, bool invalidate )
{
    if ( ( d->md & Delete ) != Delete )
	return 0;
    int k = count();
    if( k == 0 ) return 0;
    QString str = "delete from " + name();
    if ( filter.length() )
	str+= " where " + filter;
    return apply( str, invalidate );
}

/*
  \internal
*/

int QSqlCursor::apply( const QString& q, bool invalidate )
{
    int ar = 0;
    if ( invalidate ) {
	d->lastAt = QSql::BeforeFirst;
	if ( exec( q ) )
	    ar = numRowsAffected();
    } else if ( driver() ) {
	QSqlQuery sql( driver()->createQuery() );
	if ( sql.exec( q ) )
	    ar = sql.numRowsAffected();
    }
    return ar;
}

/*!  \reimp

  Executes the SQL query \a sql.  Returns TRUE of the cursor is
  active, otherwise returns FALSE.

*/
bool QSqlCursor::exec( const QString & sql )
{
    QSqlQuery::exec( sql );
    return isActive();
}

/*! Protected virtual function which is called whenever a field needs
  to be calculated.  If calculated fields are being used, derived
  classes must reimplement this function and return the appropriate
  value for field \a name.  The default implementation returns an
  invalid QVariant.

  \sa setCalculated()
*/

QVariant QSqlCursor::calculateField( const QString& )
{
    return QVariant();
}

/*! \internal
   Ensure fieldlist is synced with query.

*/

static QString qTrim( const QString& s )
{
    QString result = s;
    int end = result.length() - 1;
    while ( end >= 0 && result[end].isSpace() ) // skip white space from end
	end--;
    result.truncate( end + 1 );
    return result;
}

/*! \internal
 */

void QSqlCursor::sync()
{
    if ( isActive() && isValid() && d->lastAt != at() ) {
	d->lastAt = at();
	uint i = 0;
	uint j = 0;
	bool haveCalculatedFields = FALSE;
	for ( ; i < count(); ++i ) {
	    if ( !haveCalculatedFields && d->infoBuffer[i].isCalculated() ) {
		haveCalculatedFields = TRUE;
	    }
	    if ( QSqlRecord::isGenerated( i ) ) {
		QVariant v = QSqlQuery::value( j );
		if ( ( v.type() == QVariant::String || v.type() == QVariant::CString ) &&
			d->infoBuffer[ i ].isTrim() ) {
		    v = qTrim( v.toString() );
		}
		QSqlRecord::setValue( i, v );
		if ( QSqlQuery::isNull( j ) )
		    QSqlRecord::field( i )->setNull();
		j++;
	    }
	}
	if ( haveCalculatedFields ) {
	    for ( i = 0; i < count(); ++i ) {
		if ( d->infoBuffer[i].isCalculated() )
		    QSqlRecord::setValue( i, calculateField( fieldName( i ) ) );
	    }
	}
    }
}

/*! \reimp

*/

void QSqlCursor::afterSeek()
{
    sync();
}

/*! \reimp
  Returns the value of field number \a i.
*/

QVariant QSqlCursor::value( int i ) const
{
    return QSqlRecord::value( i );
}

/*! \reimp
  \overload

  Returns the value of the field named \a name.
*/

QVariant QSqlCursor::value( const QString& name ) const
{
    return QSqlRecord::value( name );
}

#endif
