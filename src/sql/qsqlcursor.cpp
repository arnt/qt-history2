#include "qsqlcursor.h"

#ifndef QT_NO_SQL
#include "qsqldriver.h"
#include "qsqlresult.h"
#include "qdatetime.h"
#include "qsqldatabase.h"

class QSqlCursorPrivate
{
public:
    QSqlCursorPrivate( const QString& name )
	: lastAt( QSqlResult::BeforeFirst ), nm( name ), srt( name ), md( 0 )
    {}
    int               lastAt;
    QString           nm;
    QSqlIndex         srt;
    QString           ftr;
    int               md;
    QSqlIndex         priIndx;
};

QString qOrderByClause( const QSqlIndex & i, const QString& prefix = QString::null )
{
    QString str;
    int k = i.count();
    if( k == 0 ) return QString::null;
    str = " order by " + i.toString( prefix );
    return str;
}

/*!
  Constructs a cursor on database \a db.  If \a autopopulate is TRUE, the \a name of the cursor
  must correspond to an existing table or view name in the database so that field information
  can be automatically created..

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
  Constructs a copy of cursor \a s.

*/

QSqlCursor::QSqlCursor( const QSqlCursor & s )
    : QSqlRecord( s ), QSqlQuery( s )
{
    d = new QSqlCursorPrivate( s.d->nm );
    d->lastAt = s.d->lastAt;
    d->nm = s.d->nm;
    d->srt = s.d->srt;
    d->ftr = s.d->ftr;
    d->priIndx = s.d->priIndx;
    setMode( s.mode() );
}

QSqlCursor::~QSqlCursor()
{
    delete d;
}

/*!
  Sets the cursor equal to \a s.

*/

QSqlCursor& QSqlCursor::operator=( const QSqlCursor& s )
{
    QSqlRecord::operator=( s );
    QSqlQuery::operator=( s );
    if ( d )
	delete d;
    d = new QSqlCursorPrivate( s.d->nm );
    d->lastAt = s.d->lastAt;
    d->nm = s.d->nm;
    d->srt = s.d->srt;
    d->ftr = s.d->ftr;
    d->priIndx = s.d->priIndx;
    setMode( s.mode() );
    return *this;
}

/*!  Returns the current sort, or an empty index if there is no
  current sort.

*/
QSqlIndex QSqlCursor::sort() const
{
    return d->srt;
}

/*!  Returns the current filter, or an empty string if there is no
  current filter.

*/
QString QSqlCursor::filter() const
{
    return d->ftr;
}

/*!  Sets the name of the cursor to \a name.  If autopopulate is TRUE,
  the \a name must correspond to a valid table or view name in the
  database.

*/
void QSqlCursor::setName( const QString& name, bool autopopulate )
{
    d->nm = name;
    if ( autopopulate ) {
	*this = driver()->record( name );
	d->priIndx = driver()->primaryIndex( name );
#ifdef QT_CHECK_RANGE
	if ( !count() )
	    qWarning("QSqlCursor::setName: unable to build record, does %s exist?", name.latin1() );
#endif	
    }
}

/*!  Returns the name of the cursor.

*/

QString QSqlCursor::name() const
{
    return d->nm;
}

/*!
  \internal

*/
QSqlRecord & QSqlCursor::operator=( const QSqlRecord & list )
{
    return QSqlRecord::operator=( list );
}

/*!  Returns the primary index associated with the cursor, or an empty
  index if there is no primary index.  If \a prime is TRUE, the index
  fields are set with the current value of the cursor fields they
  correspond to.

*/

QSqlIndex QSqlCursor::primaryIndex( bool prime ) const
{
    if ( prime ) {
	for ( uint i = 0; i < d->priIndx.count(); ++i ) {
	    const QString fn = d->priIndx.field( i )->name();
	    d->priIndx.field( i )->setValue( field( fn )->value() );
	}
    }
    return d->priIndx;
}

/*!  Sets the primary index associated with the cursor.  Note that this
  index must be able to identify a unique record within the underlying
  table or view.

*/

void QSqlCursor::setPrimaryIndex( const QSqlIndex& idx )
{
    d->priIndx = idx;
}

/*!  Returns an index compromised of \a fieldNames, or an empty index if
  the field names do not exist. The index is returned with all field
  values primed with the values of the cursor fields they correspond to.

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

/*!  Returns an index compromised of a single \a fieldName, or an empty index if
  the field name does not exist. The index is returned with the field
  value primed with the value of the cursor field it corresponds to.

*/

QSqlIndex QSqlCursor::index( const QString& fieldName ) const
{
    QSqlIndex idx;
    const QSqlField* f = field( fieldName );
    if ( f )
	idx.append( *f );
    return idx;
}

/*

*/

QSqlIndex QSqlCursor::index( const char* fieldName ) const
{
    return index( QString( fieldName ) );
}

/*!
  Selects all fields in the cursor.  The order in which the data is returned
  is database-specific.

*/

bool QSqlCursor::select()
{
    return select( "*", QSqlIndex() );
}

/*!
  Selects all fields in the cursor.  The data is returned in the order specified
  by the index \a sort.

*/

bool QSqlCursor::select( const QSqlIndex& sort )
{
    return select( "*", sort );
}

/*!
  Selects all fields in the cursor matching the filter criteria \a filter.  The
  data is returned in the order specified by the index \a sort.  Note that the
  \a filter string will be placed in the generated WHERE clause, but should not
  include the 'WHERE' keyword.  As a special case, using "*" as the filter string
  will retrieve all records.  For example:

  \code
  QSqlCursor myCursor(db, "MyTable");
  myCursor.select("deptID=10"); // select everything in department 10
  ...
  myCursor.select("*"); // select all records in cursor
  ...
  myCursor.select("deptID>10");  // select other departments
  ...
  myCursor.select(); // select all records again
  \endcode

*/

bool QSqlCursor::select( const QString & filter, const QSqlIndex & sort )
{
    QString str= "select " + toString( d->nm );
    str += " from " + d->nm;
    if ( !filter.isNull() && filter != "*" ) {
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

/*!
  Selects all fields in the cursor matching the filter index \a filter.  The
  data is returned in the order specified by the index \a sort.  Note that the
  \a filter index fields that are in the cursor will use the current value of the cursor
  data fields when generating the WHERE clause.  This method is useful, for example,
  for retrieving data based upon a table's primary index:

  \code
  QSqlCursor myCursor(db, "MyTable");
  QSqlIndex pk = db->primaryIndex("MyTable");
  myCursor["id"] = 10;
  myCursor.select( pk ); // generates "select ... from MyTable where id=10;"
  ...
  \endcode

*/
bool QSqlCursor::select( const QSqlIndex & filter, const QSqlIndex & sort )
{
    return select( fieldEqualsValue( d->nm, "and", filter ), sort );
}

/*!
  Sets the cursor mode to \a mode.  This value can be an OR'ed
  combination of QSqlCursor Modes.

  For example,

  \code
  QSqlCursor cursor( "emp" );
  cursor.setMode( QSqlCursor::Writeable ); // allow insert/update/delete
  ...
  cursor.setMode( QSqlCursor::Insert | QSqlCursor::Update ); // allow inserts and updates
  ...
  cursor.setMode( QSqlCursor::ReadOnly ); // no inserts/updates/deletes allowed

  \endcode
*/

void QSqlCursor::setMode( int mode )
{
    d->md = mode;
}

/*
   Returns the current cursor mode.

   \sa setMode
*/

int QSqlCursor::mode() const
{
    return d->md;
}

/*
   Returns TRUE if the cursor is read-only, FALSE otherwise.

   \sa setMode
*/

bool QSqlCursor::isReadOnly() const
{
    return d->md == 0;
}

/*
   Returns TRUE if the cursor will perform inserts, FALSE otherwise.

   \sa setMode
*/

bool QSqlCursor::canInsert() const
{
    return ( ( d->md & Insert ) == Insert ) ;
}


/*
   Returns TRUE if the cursor will perform updates, FALSE otherwise.

   \sa setMode
*/

bool QSqlCursor::canUpdate() const
{
    return ( ( d->md & Update ) == Update ) ;
}

/*
   Returns TRUE if the cursor will perform updates, FALSE otherwise.

   \sa setMode
*/


bool QSqlCursor::canDelete() const
{
    return ( ( d->md & Update ) == Update ) ;
}



QString qMakeFieldValue( const QSqlDriver* driver, const QString& prefix, QSqlField* field, const QString& op = "=" )
{
    QString f = ( prefix.length() > 0 ? prefix + QString(".") : QString::null ) + field->name();
    f += " " + op + " " + driver->formatValue( field );
    return f;
}

/*!
  \internal

*/
QString QSqlCursor::fieldEqualsValue( const QString& prefix, const QString& fieldSep, const QSqlIndex & i )
{
    QString filter;
    int k = i.count();

    if ( k ) { // use index
	for( int j = 0; j < k; ++j ){
	    if( j > 0 )
		filter += " " + fieldSep + " " ;
	    QString fn = i.field(j)->name();
	    QSqlField* f = field( fn );
	    filter += qMakeFieldValue( driver(), prefix, f );
	}
    } else { // use all fields
 	for ( uint j = 0; j < count(); ++j ) {
	    if ( j > 0 )
		filter += " " + fieldSep + " " ;
	    filter += qMakeFieldValue( driver(), prefix, field( j ) );
	}
    }
    return filter;
}

/*!  Inserts the current contents of the cursor record buffer into the
  database, if the cursor allows inserts.  If \a invalidate is TRUE, the
  current cursor can no longer be navigated (i.e., any prior select
  statements will no longer be active or valid).  Returns the number
  of rows affected by the insert.  For error information, use
  lastError().

  \sa setMode
*/

int QSqlCursor::insert( bool invalidate )
{
    if ( ( d->md & Insert ) != Insert )
	return FALSE;
    int k = count();
    if( k == 0 ) return 0;
    QString str = "insert into " + name();
    str += " (" + QSqlRecord::toString() + ")";
    str += " values (";
    QString vals;
    for( int j = 0; j < k; ++j ){
	if( j > 0 )
	    vals += ",";
	vals += driver()->formatValue( field(j) );
    }
    str += vals + ");";
    return apply( str, invalidate );
}

/*!  Updates the database with the current contents of the record
  buffer, using the specified \a filter.  Only records which meet the
  filter criteria are updated, otherwise all records in the table are
  updated.  If \a invalidate is TRUE, the current cursor can no longer
  be navigated (i.e., any prior select statements will no longer be
  active or valid). Returns the number of records which were updated.
  For error information, use lastError().

*/

int QSqlCursor::update( const QString & filter, bool invalidate )
{
    if ( ( d->md & Update ) != Update )
	return FALSE;
    int k = count();
    if( k == 0 ) return 0;
    QString str = "update " + name();
    str += " set " + fieldEqualsValue( "", "," );
    if ( filter.length() )
 	str+= " where " + filter;
    str += ";";
    return apply( str, invalidate );
}

/*!  Updates the database with the current contents of the record
  buffer, using the filter index \a filter.  Only records which meet
  the filter criteria specified by the index are updated.  If no index
  is specified, the primary index of the underlying cursor is used.
  If \a invalidate is TRUE, the current cursor can no longer be
  navigated (i.e., any prior select statements will no longer be
  active or valid). Returns the number of records which were
  updated. For error information, use lastError().  For example:

  \code
  QSqlDatabase* db;
  ...
  QSqlCursor empCursor ( db, "Employee" );
  empCursor["id"] = 10;  // set the primary index field
  empCursor["firstName"] = "Dave";
  empCursor.update();  // update an employee name using primary index
  \endcode

*/

int QSqlCursor::update( const QSqlIndex & filter, bool invalidate )
{
    return update( fieldEqualsValue( "", "and", filter ), invalidate );
}

/*!  Deletes the record from the cursor using the filter \a filter.
  Only records which meet the filter criteria specified by the index
  are deleted.  Returns the number of records which were updated. If
  \a invalidate is TRUE, the current cursor can no longer be navigated
  (i.e., any prior select statements will no longer be active or
  valid). For error information, use lastError().

*/

int QSqlCursor::del( const QString & filter, bool invalidate )
{
    if ( ( d->md & Delete ) != Delete )
	return FALSE;
    int k = count();
    if( k == 0 ) return 0;
    QString str = "delete from " + name();
    if ( filter.length() )
 	str+= " where " + filter;
    str += ";";
    return apply( str, invalidate );
}

/*!  Deletes the record from the cursor using the filter index \a
  filter.  Only records which meet the filter criteria specified by
  the index are updated.  If no index is specified, the primary index
  of the underlying cursor is used.  If \a invalidate is TRUE, the
  current cursor can no longer be navigated (i.e., any prior select
  statements will no longer be active or valid). Returns the number of
  records which were deleted.  For error information, use lastError().
  For example:

*/

int QSqlCursor::del( const QSqlIndex & filter, bool invalidate )
{
    return del( fieldEqualsValue( "", "and", filter ), invalidate );
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
    } else {
	QSqlQuery sql( driver()->createQuery() );
	if ( sql.exec( q ) )
	    ar = sql.numRowsAffected();
    }
    return ar;
}

/*!
  Executes the SQL query \a str.  Returns TRUE of the cursor is active,
  otherwise returns FALSE.

*/
bool QSqlCursor::exec( const QString & str )
{
    QSqlQuery::exec( str );
    return isActive();
}

QVariant QSqlCursor::calculateField( uint )
{
    return QVariant();
}

void QSqlCursor::detach()
{
    exec( QString::null );
}

/*
  \internal

  Make sure fieldlist is synced with sql.

*/
void QSqlCursor::sync()
{
    if ( isActive() && isValid() && d->lastAt != at() ) {
	d->lastAt = at();
	uint i = 0;
	for ( ; i < count(); ++i ){
	    if ( field(i)->isCalculated() )
		QSqlRecord::setValue( i, calculateField( i ) );
	    else {
		QSqlRecord::setValue( i, QSqlQuery::value(i) );
		QSqlRecord::field( i )->setNull( QSqlQuery::isNull( i ) );
	    }
	}
    }
}

/*!
  \reimpl
*/

void QSqlCursor::afterSeek()
{
    sync();
}

/*!
  \reimpl
*/

QVariant QSqlCursor::value( int i )
{
    return QSqlRecord::value( i );
}

/*!
  \reimpl
*/

QVariant QSqlCursor::value( const QString& name )
{
    return QSqlRecord::value( name );
}

#endif

