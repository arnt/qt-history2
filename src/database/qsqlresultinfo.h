#ifndef QSQLRESULTINFO_H
#define QSQLRESULTINFO_H

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#include "qvaluelist.h"
#include "qshared.h"
#endif // QT_H

#ifndef QT_NO_SQL

struct QSqlFieldInfo
{
    QSqlFieldInfo( const QString& fieldName,
		   QVariant::Type fieldType=QVariant::Invalid,
		   int fieldLength=0,
		   int fieldPrecision=0)
	: name(fieldName), type(fieldType), length(fieldLength), precision(fieldPrecision)
    {}
    QSqlFieldInfo(){}
    QString name;
    int     type;
    int     length;
    int     precision;
#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
    bool operator==( const QSqlFieldInfo& ) const { return FALSE; }
#endif
};

typedef QValueList<QSqlFieldInfo> QSqlFieldInfoList;

class QSqlResultInfo
{
public:
    virtual	    ~QSqlResultInfo();
    QSqlFieldInfoList fields() const;
    virtual int     size() const;
    virtual int     affectedRows() const;
protected:
    QSqlResultInfo();
    QSqlResultInfo( const QSqlResultInfo & );
    QSqlResultInfo& operator=( const QSqlResultInfo & );
    int 	    appendField( const QSqlFieldInfo& field );
    void            setSize( int size );
    void            setAffectedRows( int rows );
private:
    int 	    sz;
    int 	    affRows;
    QSqlFieldInfoList fieldList;
};

#endif	// QT_NO_SQL
#endif
