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
#include "qmap.h"

class QSqlCursorPrivate
{
public:
    QSqlCursorPrivate( const QString& name )
	: lastAt( QSqlResult::BeforeFirst ), nm( name ), srt( name ), md( 0 ){}
    ~QSqlCursorPrivate(){}

    int               lastAt;
    QString           nm;
    QSqlIndex         srt;
    QString           ftr;
    int               md;
    QSqlIndex         priIndx;
    QSqlRecord        editBuffer;
    QMap< int, bool >  calcFields;
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

    \brief A database cursor for browsing and editing SQL tables and views.

    \module sql

    A 'cursor' is a reference to a database record (see \l QSqlRecord)
    that corresponds to a table or view within an SQL database.  Cursors
    contain a list of fields, and when positioned on a valid record,
    contain the value's for the record's fields. Cursors can be used
    to browse the database, to edit existing records, and to add new
    records.

    To position the cursor on a valid record, cursors can be navigated
    in the same way as a \l QSqlQuery, using next(), first(), seek(),
    etc.  Once positioned on a valid record, data can be retrieved from
    the record's fields.  In addition, for cursors which correspond to
    tables or views that contain primary indexes, data can be edited
    using the edit functions insert(), update() and del().

    To edit a database record, edit the values in the cursor's edit
    buffer (see editBuffer() and primeUpdate()).  The edit buffer can
    then be updated in the database. To add a new record populate an
    empty edit buffer (see editBuffer() and primeInsert()) and then
    update the database with the new record.

    Many operations apply to the "current cursor record". If the cursor
    has never been positioned on a valid record, e.g. immediately after
    creation, then the field values it returns are all nulls. If the
    cursor is positioned on a valid record, e.g. after a next(),
    first(), last(), prev() or seek() that succeeded (isValid() returns
    TRUE), then the field values it returns are those of the record it
    is positioned on. If the cursor is moved to an invalid record, e.g.
    after an update(), insert() or del(), then the field values returned
    are those of the last valid record it was positioned on.

*/

/*! \enum QSqlCursor::Mode

  This enum type describes how QSqlCursor operates on records in the
  database.

  The currently defined values are:
  <ul>

  <li> \c ReadOnly - the cursor can only select records from the
  database.

  <li> \c Insert - the cursor can insert records into the database.

  <li> \c Update - the cursor can update records in the database.

  <li> \c Delete - the cursor can delete records from the database.

  <li> \c Writable - the cursor can insert, update and delete records
  in the database.

  </ul>

*/

/*!  Constructs a cursor on database \a db.  If \a autopopulate is
  TRUE (the default), the \a name of the cursor must correspond to an
  existing table or view name in the database so that field
  information can be automatically created.  If the table or view does
  not exist invalid SQL will be generated. The cursor is created with an
  initial mode of QSqlCursor::Writable (meaning that records can be
  inserted, updated or deleted using the cursor). Note that \a
  autopopulate refers to populating the cursor with meta-data, e.g. the
  names of the table's fields, not with retrieving data. The refresh()
  function is used to populate the cursor with data. 

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
    if ( d )
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
  selected.  To select new records, use select(). This sort will apply
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
  are selected.  To select new records, use select(). This filter will
  apply to any subsequent select() calls that do not explicitly specify
  a filter. 


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

/*!  Sets the name of the cursor to \a name.  If autopopulate is TRUE
  (the default), the \a name must correspond to a valid table or view
  name in the database.

*/
void QSqlCursor::setName( const QString& name, bool autopopulate )
{
    d->nm = name;
    if ( autopopulate ) {
	if ( driver() ) {
	    d->editBuffer = driver()->record( name );
	    *this = d->editBuffer;
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

    for ( uint i = 0; i < count(); ++i ){
	const QString fname = fieldName( i );
	if ( !isCalculated( fname ) && isGenerated( fname ) ) {
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

*/
QSqlRecord & QSqlCursor::operator=( const QSqlRecord & list )
{
    return QSqlRecord::operator=( list );
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

/*!  Sets the primary index associated with the cursor.  Note that
  this index must contain fields which identify a unique record within
  the underlying database table or view.

*/

void QSqlCursor::setPrimaryIndex( const QSqlIndex& idx )
{
    d->priIndx = idx;
}


/*!  Returns an index composed of \a fieldNames.  Note that all field
  names must exist in the cursor, otherwise an empty index is
  returned.
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
*/

QSqlIndex QSqlCursor::index( const QString& fieldName ) const
{
    QStringList fl( fieldName );
    return index( fl );
}

/*! \overload
*/

QSqlIndex QSqlCursor::index( const char* fieldName ) const
{
    return index( QStringList( QString( fieldName ) ) );
}

/*!  Selects all fields in the cursor from the database matching the
  filter criteria \a filter.  The data is returned in the order
  specified by the index \a sort.  Note that the \a filter string will
  be placed in the generated WHERE clause, but should \e not include the
  'WHERE' keyword.  As a special case, using "*" as the filter string
  will retrieve all records.  The cursor is initially positioned at an
  invalid row.  To move to a valid row, use seek(), first(),
  last(), prev() or next(). For example:

  \code
  QSqlCursor myCursor( "Employee" );
  myCursor.select( "deptno=10" ); // select everything in department 10
  ...
  myCursor.select( "*" );         // select all records in cursor
  ...
  myCursor.select( "deptno>10" ); // select other departments
  ...
  myCursor.select();              // select WHERE DEPTNO>10 again
  \endcode

  The filter will apply to any subsequent select() calls that do not
  explicitly specify a filter. Similarly the sort will apply to any
  subsequent select() calls that do not explicitly specify a sort.

*/

bool QSqlCursor::select( const QString & filter, const QSqlIndex & sort )
{
    QString str= "select " + toString( d->nm );
    str += " from " + d->nm;
    if ( !filter.isEmpty() && filter != "*" ) {
	d->ftr = filter;
	str += " where " + filter;
    } else
	d->ftr = QString::null;
    if ( sort.count() > 0 )
	str += " order by " + sort.toString( d->nm );
    str += ";";
    d->srt = sort;
    d->lastAt = QSqlResult::BeforeFirst;
    return exec( str );
}

/*!  \overload

  Selects all fields in the cursor from the database.  The rows are
  returned in the order specified by the last call to setSort() or the
  last call to select() that specified a sort, whichever is the most
  recent.  If there is no current sort, the order in which the rows are
  returned is undefined.  The records are filtered according to the
  filter specified by the last call to setFilter() or the last call to
  select() that sepcified a filter, whichever is the most recent. If
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
  returned in the order specified by the index \a sort. The cursor is
  initially positioned at an invalid row.  To move to a valid row, use
  seek(), first(), last(), prev() or next().

*/

bool QSqlCursor::select( const QSqlIndex& sort )
{
    return select( "*", sort );
}

/*! \overload

  Selects all fields in the cursor matching the filter index \a filter.
  The data is returned in the order specified by the index \a sort. The
  \a filter index works by constructing a WHERE clause taking the names
  of the fields from the filter and their values from the current cursor
  record. The cursor is initially positioned at an invalid row.  To move
  to a valid row, use seek(), first(), last(), prev() or next().  This
  method is useful, for example, for retrieving data based upon a
  table's primary index:

  \code
  QSqlCursor myCursor( "Employee" );
  QSqlIndex pk = myCursor.primaryIndex();
  myCursor.setValue( "id", 10 );
  myCursor.select( pk, pk ); // generates "select ... from Employee where id=10 order by id;"
  ...
  \endcode

  In this example the QSqlIndex, pk, is used for two different purposes.
  When used as the filter argument, the field names it contains are used
  for the WHERE clause, each set to the current cursor value, "WHERE
  id=10" in this case. When used as the sort argument the field names it
  contains are used for the ORDER BY clause, "ORDER BY id" in this
  example.

*/
bool QSqlCursor::select( const QSqlIndex & filter, const QSqlIndex & sort )
{
    return select( toString( filter, this, d->nm, "=", "and" ), sort );
}

/*!  Sets the cursor mode to \a mode.  This value can be an OR'ed
  combination of QSqlCursor::Mode values. The default mode for a
  cursor is QSqlCursor::Writable.

  \code
  QSqlCursor cursor( "Employee" );
  cursor.setMode( QSqlCursor::Writeable ); // allow insert/update/delete
  ...
  cursor.setMode( QSqlCursor::Insert | QSqlCursor::Update ); // allow inserts and updates only
  ...
  cursor.setMode( QSqlCursor::ReadOnly ); // no inserts/updates/deletes allowed

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

/*! Sets field \a name to \a calculated.  If the field \a name does not
  exist, nothing happens.  The value of a calculated field is set by the
  calculateField() virtual function which you must reimplement otherwise
  the field value will be an invalid QVariant. Calculated fields do not
  appear in the generated SQL statements sent to the database. 

  \sa calculateField() QSqlRecord::setGenerated()
*/

void QSqlCursor::setCalculated( const QString& name, bool calculated )
{
    if ( !field( name ) )
	return;
    d->calcFields[ position( name ) ] = calculated;
    setGenerated( name, !calculated );
}

/*! Returns TRUE if the field \a name is calculated, otherwise FALSE is
  returned. If the field \a name does not exist, FALSE is returned.
*/

bool QSqlCursor::isCalculated( const QString& name ) const
{
    if ( !field( name ) )
	return FALSE;
    return d->calcFields[ position( name ) ];
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

/*! Returns a formatted string composed of the \a prefix (e.g. table or
    view name), ".", the \a field name, the \a fieldSep and the field
    value. If the \a prefix is empty then the string will begin with the
    \a field name. This method is useful for generating SQL statements.

*/

QString QSqlCursor::toString( const QString& prefix, QSqlField* field, const QString& fieldSep ) const
{
    QString f;
    if ( field && driver() ) {
	f = ( prefix.length() > 0 ? prefix + QString(".") : QString::null ) + field->name();
	f += " " + fieldSep + " " + driver()->formatValue( field );
    }
    return f;
}

/*! Returns a formatted string composed of all the fields in \a rec.
    Each field is composed of the \a prefix (e.g. table or view name),
    ".", the \a field name, the \a fieldSep and the field value. If the
    \a prefix is empty then the field will begin with the \a field name.
    The fields are then joined together separated by the \a sep.
    Calculated fields and fields where isGenerated() returns FALSE are
    not included. This method is useful for generating SQL statements.


*/

QString QSqlCursor::toString( QSqlRecord* rec, const QString& prefix, const QString& fieldSep,
			      const QString& sep ) const
{
    QString filter;
    bool separator = FALSE;
    for ( uint j = 0; j < count(); ++j ) {
	QSqlField* f = rec->field( j );
	if ( !isCalculated( f->name() ) && isGenerated( f->name() ) ) {
	    if ( separator )
		filter += " " + sep + " " ;
	    filter += toString( prefix, f, fieldSep );
	    separator = TRUE;
	}
    }
    return filter;
}

/*! Returns a formatted string composed of all the fields in the index
    \a i. Each field is composed of the \a prefix (e.g. table or view
    name), ".", the \a field name, the \a fieldSep and the field value.
    If the \a prefix is empty then the field will begin with the \a
    field name. The field values are taken from \a rec. The fields are
    then joined together separated by the \a sep. This method is useful
    for generating SQL statements.

*/

QString QSqlCursor::toString( const QSqlIndex& i, QSqlRecord* rec, const QString& prefix,
				const QString& fieldSep, const QString& sep ) const
{
    QString filter;
    bool separator = FALSE;
    for( uint j = 0; j < i.count(); ++j ){
	if( separator )
	    filter += " " + sep + " " ;
	QString fn = i.fieldName( j );
	QSqlField* f = rec->field( fn );
	filter += toString( prefix, f, fieldSep );
	separator = TRUE;
    }
    return filter;
}

/*! Inserts the current contents of the cursor's edit record buffer
    into the database, if the cursor allows inserts.  If \a invalidate
    is TRUE (the default), the cursor will no longer be positioned on a
    valid record and can no longer be navigated. A new select() call
    must be made before you can move to a valid record. Returns the
    number of rows affected by the insert.  For error information, use
    lastError().

  \sa setMode() lastError()
*/

int QSqlCursor::insert( bool invalidate )
{
    if ( ( d->md & Insert ) != Insert || !driver() )
	return FALSE;
    int k = d->editBuffer.count();
    if( k == 0 ) return 0;
    QString str = "insert into " + name();
    str += " (" + d->editBuffer.toString() + ")";
    str += " values (";
    QString vals;
    bool comma = FALSE;
    for( int j = 0; j < k; ++j ){
	QSqlField* f = d->editBuffer.field( j );
	if ( !isCalculated( f->name() ) ) {
	    if( comma )
		vals += ",";
	    vals += driver()->formatValue( f );
	    comma = TRUE;
	}
    }
    str += vals + ");";
    if ( invalidate )
	QSqlRecord::operator=( d->editBuffer );
    return apply( str, invalidate );
}

/*!  Returns a pointer to the current internal edit buffer.  The edit
  buffer is valid as long as the cursor remains valid.

  \sa primeInsert(), primeUpdate()
*/

QSqlRecord* QSqlCursor::editBuffer( )
{
    return &d->editBuffer;
}

/*!  'Primes' the field values of the edit buffer for update and
  returns a pointer to the edit buffer.  The default implementation
  copies the field values from the current cursor record into the edit
  buffer.

  \sa editBuffer() update()

*/

QSqlRecord* QSqlCursor::primeUpdate()
{
    if( d->editBuffer.count() == 0 ){
	d->editBuffer = *((QSqlRecord*)this);
    } else {
	for(uint i = 0; i < d->editBuffer.count(); i++){
	    d->editBuffer.setValue( i, value( i ) );
	}
    }
    return &d->editBuffer;
}

/*!  'Primes' the field values of the edit buffer for insert and
  returns a pointer to the edit buffer.  The default implementation
  clears all field values in the edit buffer.

  \sa editBuffer() insert()

*/

QSqlRecord* QSqlCursor::primeInsert()
{
    if( d->editBuffer.count() == 0 )
	d->editBuffer = *((QSqlRecord*)this);
    d->editBuffer.clearValues();
    return &d->editBuffer;
}


/*!  Updates the database with the current contents of the edit
  buffer.  Only records which meet the filter criteria specified by
  the cursor's primary index are updated.  If the cursor does not
  contain a primary index, no update is performed and 0 is returned.
  If \a invalidate is TRUE (the default), the current cursor can no
  longer be navigated. A new select() call must be made before you can
  move to a valid record. Returns the number of records which were
  updated, or 0 if there was an error (for example, if the cursor has no
  primary index). For error information, use lastError().  For example:

  \code

  QSqlCursor empCursor ( "Employee" );
  empCursor.select( "id=10");
  if ( empCursor.next() ) {
      QSqlRecord* buf = empCursor.primeUpdate();
      buf->setValue( "forename", "Dave" );
      empCursor.update();  // update employee name using primary index
  }

  \endcode

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
  buffer, using the specified \a filter.  Only records which meet the
  filter criteria are updated, otherwise all records in the table are
  updated.  If \a invalidate is TRUE (the default), the cursor can no
  longer be navigated. A new select() call must be made before you can
  move to a valid record. Returns the number of records which were
  updated.  For error information, use lastError().

  \sa setMode() lastError()
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
    str += ";";
    if ( invalidate )
	QSqlRecord::operator=( d->editBuffer );
    return apply( str, invalidate );
}

/*!  Deletes the current cursor record from the database using the
  cursor's primary index.  Only records which meet the filter criteria
  specified by the cursor's primary index are deleted.  If the cursor
  does not contain a primary index, or if the cursor is not positioned
  on a valid record, no delete is performed and 0 is returned. If \a
  invalidate is TRUE (the default), the current cursor can no longer
  be navigated. A new select() call must be made before you can move to
  a valid record. Returns the number of records which were deleted.  For
  error information, use lastError(). For example:

  \code

  QSqlCursor empCursor ( "Employee" );
  empCursor.select( "id=10");
  if ( empCursor.next() )
      empCursor.del();  // delete employee #10

  \endcode

  \sa setMode() lastError()
*/

int QSqlCursor::del( bool invalidate )
{
    if ( !isActive() || !isValid() ||  primaryIndex().isEmpty() )
	return 0;
    return del( toString( primaryIndex(), this, d->nm, "=", "and" ), invalidate );
}

/*! \overload

   Deletes the current cursor record from the database using the
   filter \a filter.  Only records which meet the filter criteria are
   deleted.  Returns the number of records which were deleted. If \a
   invalidate is TRUE (the default), the current cursor can no longer
   be navigated. A new select() call must be made before you can move to
   a valid record. For error information, use lastError().

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
    str += ";";
    if ( invalidate )
	clearValues();
    return apply( str, invalidate );
}

/*
  \internal
*/

int QSqlCursor::apply( const QString& q, bool invalidate )
{
    int ar = 0;
    if ( invalidate ) {
	d->lastAt = QSqlResult::BeforeFirst;
	if ( exec( q ) )
	    ar = numRowsAffected();
    } else if ( driver() ) {
	QSqlQuery sql( driver()->createQuery() );
	if ( sql.exec( q ) )
	    ar = sql.numRowsAffected();
    }
    return ar;
}

/*!  Executes the SQL query \a str.  Returns TRUE of the cursor is
  active, otherwise returns FALSE.

*/
bool QSqlCursor::exec( const QString & sql )
{
    QSqlQuery::exec( sql );
    return isActive();
}

/*! Protected virtual function which is called whenever a field needs
  to be calculated.  Derived classes should reimplement this function
  and return the appropriate value for field \a name.  The default
  implementation returns an invalid QVariant.

  \sa setCalculated()
*/

QVariant QSqlCursor::calculateField( const QString& )
{
    return QVariant();
}

/*! \internal
   Ensure fieldlist is synced with query.

*/

void QSqlCursor::sync()
{
    if ( isActive() && isValid() && d->lastAt != at() ) {
	d->lastAt = at();
	uint i = 0, j = 0;
	for ( ; i < count(); ++i ){
	    QSqlField* f = field( i );
	    if ( !isCalculated( f->name() ) ){
		QSqlRecord::setValue( i, QSqlQuery::value( j ) );
		QSqlRecord::field( i )->setNull( QSqlQuery::isNull( j ) );
		j++;
	    }
	}
	i = 0;
	for ( ; i < count(); ++i ){
	    QSqlField* f = field( i );
	    if ( isCalculated( f->name() ) )
		QSqlRecord::setValue( i, calculateField( f->name() ) );
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
*/

QVariant QSqlCursor::value( int i ) const
{
    return QSqlRecord::value( i );
}

/*! \reimp
*/

QVariant QSqlCursor::value( const QString& name ) const
{
    return QSqlRecord::value( name );
}

#endif
