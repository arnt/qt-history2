#include "qsqlquerybuilder.h"
#include "qmap.h"

class QSqlQueryBuilderPrivate {
public:
    QSqlQueryBuilderPrivate() {}
    QString baseTable;
    QMap<QString,QStringList> tableFields;
    QMap<QString,QStringList> indices;
    QStringList joinTables;
    QStringList joinConditions;
    QValueList<QSqlQueryBuilder::JoinType> joinTypes;
};

static QString qTableFieldList( const QString& table, const QStringList& strl )
{
    bool comma = FALSE;
    QString fields;
    QStringList::ConstIterator it;
    for ( it = strl.begin(); it != strl.end(); ++it ) {
	if ( comma ) {
	    fields += ", " + table + "." + *it;
	} else {
	    fields += table + "." + *it;
	    comma = TRUE;
	}
    }
    return fields;
}

static QString qIndexList( const QString& table, const QStringList& index )
{
    bool comma = FALSE;
    QString i;
    QStringList::ConstIterator it;
    for ( it = index.begin(); it != index.end(); ++it ) {
	if ( comma ) {
	    i += " and " + *it + " = :" + table + "_" + *it;
	} else {
	    i += *it + " = :" + table + "_" + *it;
	    comma = TRUE;
	}
    }
    
    return i;    
}

QSqlQueryBuilder::QSqlQueryBuilder( const QString& table )
{
    d = new QSqlQueryBuilderPrivate;
    d->baseTable = table;
}

QSqlQueryBuilder::QSqlQueryBuilder( const QSqlQueryBuilder& other )
{
    d = new QSqlQueryBuilderPrivate;
    d->tableFields = other.d->tableFields;
    d->joinTables = other.d->joinTables;
    d->joinConditions = other.d->joinConditions;
    d->joinTypes = other.d->joinTypes;
    d->baseTable = other.d->baseTable;
}

QSqlQueryBuilder::~QSqlQueryBuilder()
{
    delete d;
}

QSqlQueryBuilder& QSqlQueryBuilder::join( const QString& table, const QString& joinCondition )
{
    return doJoin( table, joinCondition, Natural );
}

QSqlQueryBuilder& QSqlQueryBuilder::leftOuterJoin( const QString& table, const QString& joinCondition )
{
    return doJoin( table, joinCondition, LeftOuter );
}

QSqlQueryBuilder& QSqlQueryBuilder::rightOuterJoin( const QString& table, const QString& joinCondition )
{
    return doJoin( table, joinCondition, RightOuter );
}

QSqlQueryBuilder& QSqlQueryBuilder::fullOuterJoin( const QString& table, const QString& joinCondition )
{
    return doJoin( table, joinCondition, FullOuter );
}

QSqlQueryBuilder& QSqlQueryBuilder::doJoin( const QString& table, const QString& joinCondition, QSqlQueryBuilder::JoinType type )
{
    d->joinTables.append( table );
    d->joinConditions.append( joinCondition );
    d->joinTypes.append( type );
    return *this;
}

QString QSqlQueryBuilder::selectQuery() const
{
    // use ANSI join syntax
    int i = 0;
    bool comma = FALSE;
    QMapConstIterator<QString, QStringList> it;
    QString tmp;
    QString q;
    q = "select ";
    // field list
    for ( it = d->tableFields.begin(); it != d->tableFields.end(); ++it ) {
	if ( comma ) {
	    q += ", " + qTableFieldList( it.key(), it.data() );
	} else {
	    q += qTableFieldList( it.key(), it.data() );
	    comma = TRUE;
	}
    }
    q += " from " + d->baseTable;
    QStringList::ConstIterator sit;
    for ( sit = d->joinTables.begin(); sit != d->joinTables.end(); ++sit ) {
	switch ( d->joinTypes[i] ) {
	    case Natural:
		tmp = " natural join ";
		break;
	    case LeftOuter:
		tmp = " left outer join ";
		break;
	    case RightOuter:
		tmp = " right outer join ";
		break;
	    case FullOuter:
		tmp = " full outer join ";
		break;
	}
	q += tmp + *sit + " on ( " + d->joinConditions[i] + " )";
	i++;
    }
    return q;
}

QStringList QSqlQueryBuilder::insertQueries() const
{
    // insert into $table ( $f1, $f2 ...) values ( :f1, :f2 ... )
    QMapConstIterator<QString,QStringList> it;
    QStringList::ConstIterator sit;
    bool comma;
    QStringList strl;
    QString q;
    for ( it = d->tableFields.begin(); it != d->tableFields.end(); ++it ) {
	comma = FALSE;
	q = "insert into " + it.key() + " ( " + it.data().join( ", " ) + " ) values ( ";
	for ( sit = it.data().begin(); sit != it.data().end(); ++sit ) {
	    if ( comma ) {
		q += ", :" + it.key() + "_" + *sit;
	    } else {
		q += ":" + it.key() + "_" + *sit;
		comma = TRUE;
	    }
	}
	q += " )";
	strl.append( q );
    }    
    return strl;
}

QStringList QSqlQueryBuilder::updateQueries() const
{
    // update $table set $f1 = :f1 ... where primarykey = :p1
    QMapConstIterator<QString,QStringList> it;
    QStringList::ConstIterator sit;
    bool comma;
    QStringList strl;
    QString q;
    for ( it = d->tableFields.begin(); it != d->tableFields.end(); ++it ) {
	comma = FALSE;
	q = "update " + it.key() + " set ";
	for ( sit = it.data().begin(); sit != it.data().end(); ++sit ) {
	    if ( comma ) {
		q += ", " + *sit + " = :" +it.key() + "_" + *sit;
	    } else {
		q += *sit + " = :" +it.key() + "_" + *sit;
		comma = TRUE;
	    }
	}
	q += " where " + qIndexList( it.key(), d->indices[it.key()] );
	strl.append( q );
    }
    return strl;
}

QStringList QSqlQueryBuilder::deleteQueries() const
{
    // delete from $table where primarykey = :p1
    QMapConstIterator<QString,QStringList> it;
    QStringList strl;
    for ( it = d->tableFields.begin(); it != d->tableFields.end(); ++it )
	strl.append( "delete from " + it.key() + " where " + qIndexList( it.key(), d->indices[it.key()] ) );
    return strl;
}

QString QSqlQueryBuilder::table( int i ) const
{
    int y = 0;
    QMapConstIterator<QString,QStringList> it;
    for ( it = d->tableFields.begin(); it != d->tableFields.end(); ++it ) {
	if ( y == i )
	    break;
	y++;
    }
    return it.key();
}

QString QSqlQueryBuilder::joinCondition( int i ) const
{
    return d->joinConditions[i];
}

void QSqlQueryBuilder::setTable( int i, const QString& table )
{
//     d->tables[i] = table;
}

void QSqlQueryBuilder::setJoinCondition( int i, const QString& condition )
{
    d->joinConditions[i] = condition;
}

void QSqlQueryBuilder::setFieldList( const QString& table, const QStringList& fields )
{
    d->tableFields[table] = fields;
}

void QSqlQueryBuilder::addField( const QString& table, const QString& field )
{
    d->tableFields[table].append( field );
}

void QSqlQueryBuilder::setPrimaryKey( const QString& table, const QStringList& index )
{
    d->indices[table] = index;
}

void QSqlQueryBuilder::setBaseTable( const QString& table )
{
    d->baseTable = table;
}

void QSqlQueryBuilder::reset( const QString& baseTable )
{
    d->tableFields.clear();
    d->joinTables.clear();
    d->joinConditions.clear();
    d->joinTypes.clear();
    d->baseTable = baseTable;
}

int QSqlQueryBuilder::tableCount() const
{
    return d->tableFields.count();
}

QStringList QSqlQueryBuilder::fieldList( const QString& table ) const
{
    return d->tableFields[ table ];
}

QString QSqlQueryBuilder::baseTable()
{
    return d->baseTable;
}
