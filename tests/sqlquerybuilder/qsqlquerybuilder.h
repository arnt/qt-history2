#ifndef QSQLQUERYBUILDER_H
#define QSQLQUERYBUILDER_H

#ifndef QT_H
#include "qstring.h"
#include "qstringlist.h"
#endif // QT_H

#if !defined( QT_MODULE_SQL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_EXPORT_SQL
#else
#define QM_EXPORT_SQL Q_EXPORT
#endif

#ifndef QT_NO_SQL
class QSqlQueryBuilderPrivate;

class QM_EXPORT_SQL QSqlQueryBuilder
{
public:
    enum JoinType { Natural = 0, LeftOuter = 1, RightOuter = 2, FullOuter = 3 };
    
    QSqlQueryBuilder( const QString& table, bool autoPopulate = TRUE );
    QSqlQueryBuilder( const QSqlQueryBuilder& other );
    ~QSqlQueryBuilder();
    
    QSqlQueryBuilder& join( const QString& table, const QString& joinCondition );
    QSqlQueryBuilder& leftOuterJoin( const QString& table, const QString& joinCondition );
    QSqlQueryBuilder& rightOuterJoin( const QString& table, const QString& joinCondition );
    QSqlQueryBuilder& fullOuterJoin( const QString& table, const QString& joinCondition );
    
    QString selectQuery() const;
    QStringList insertQueries() const;
    QStringList updateQueries() const;
    QStringList deleteQueries() const;
    QString table( int i ) const;
    QString joinCondition( int i ) const;
    
    void setPrimaryKey( const QString& table, const QStringList& index );
    void setTable( int i, const QString& table );
    void setJoinCondition( int i, const QString& condition );
    
    void setFieldList( const QString& table, const QStringList& fields );
    void addField( const QString& table, const QString& field );
    
private:
    QSqlQueryBuilder& doJoin( const QString& table, const QString& joinCondition, QSqlQueryBuilder::JoinType type );
    QSqlQueryBuilderPrivate* d;
};

#endif // QT_NO_SQL
#endif // QSQLQUERYBUILDER_H
