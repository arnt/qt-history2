#include "qsqlquerybuilder.h"

class QSqlQueryBuilderPrivate {
public:
    QSqlQueryBuilderPrivate() :	numJoins(0) {}
    int numJoins;
    QString baseTable;
    QStringList fields;
    QStringList tables;
    QStringList joinConditions;
    QValueList<QSqlQueryBuilder::JoinType> joinTypes;
};

QSqlQueryBuilder::QSqlQueryBuilder( const QString& table, bool )
{
    d = new QSqlQueryBuilderPrivate;
    d->baseTable = table;
}

QSqlQueryBuilder::QSqlQueryBuilder( const QSqlQueryBuilder& other )
{
    d = new QSqlQueryBuilderPrivate;
    d->numJoins = other.d->numJoins;
    d->tables = other.d->tables;
    d->joinConditions = other.d->joinConditions;
    d->joinTypes = other.d->joinTypes;
}

QSqlQueryBuilder::~QSqlQueryBuilder()
{
    delete d;
}

QSqlQueryBuilder QSqlQueryBuilder::join( const QString& table, const QString& joinCondition )
{
    d->tables.append( table );
    d->joinConditions.append( joinCondition );
    d->joinTypes[ d->numJoins++ ] = Natural;
    return *this;
}

QSqlQueryBuilder QSqlQueryBuilder::leftOuterJoin( const QString& table, const QString& joinCondition )
{
    d->tables.append( table );
    d->joinConditions.append( joinCondition );
    d->joinTypes[ d->numJoins++ ] = LeftOuter;
    return *this;
}

QSqlQueryBuilder QSqlQueryBuilder::rightOuterJoin( const QString& table, const QString& joinCondition )
{
    d->tables.append( table );
    d->joinConditions.append( joinCondition );
    d->joinTypes[ d->numJoins++ ] = RightOuter;
    return *this;
}

QSqlQueryBuilder QSqlQueryBuilder::fullOuterJoin( const QString& table, const QString& joinCondition )
{
    d->tables.append( table );
    d->joinConditions.append( joinCondition );
    d->joinTypes[ d->numJoins++ ] = FullOuter;
    return *this;
}

QString QSqlQueryBuilder::selectQuery() const
{
    int i = 0;
    QValueListConstIterator<QString> it;
    QString tmp;
    QString q;
    q = "select " + d->fields.join( ", " ) + " from " + d->baseTable;
    for ( it = d->tables.begin(); it != d->tables.end(); ++it ) {
	switch( d->joinTypes[i] ) {
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
	q += tmp + d->tables[i] + " on ( " + d->joinConditions[i] + " )";
	i++;
    }
    return q;
}

QStringList QSqlQueryBuilder::insertQueries() const
{
    return QStringList();
}

QStringList QSqlQueryBuilder::updateQueries() const
{
    return QStringList();
}

QStringList QSqlQueryBuilder::deleteQueries() const
{
    return QStringList();
}

QString QSqlQueryBuilder::table( int i ) const
{
    return d->tables[i];
}

QString QSqlQueryBuilder::joinCondition( int i ) const
{
    return d->joinConditions[i];
}

void QSqlQueryBuilder::setTable( int i )
{
}

void QSqlQueryBuilder::setJoinCondition( int i )
{
}

void QSqlQueryBuilder::setFieldList( const QStringList& fields )
{
    d->fields = fields;
}

void QSqlQueryBuilder::addField( const QString& field )
{
    d->fields.append( field );
}
