#ifndef QSQLRESULTINFO_H
#define QSQLRESULTINFO_H

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#include "qvaluelist.h"
#include "qshared.h"
#include "qsqlfield.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlResultInfo
{
public:
    virtual	    ~QSqlResultInfo();
    QSqlFieldList   fields() const;
    virtual int     size() const;
    virtual int     affectedRows() const;
protected:
    QSqlResultInfo();
    QSqlResultInfo( const QSqlResultInfo & );
    QSqlResultInfo& operator=( const QSqlResultInfo & );
    int 	    appendField( const QSqlField& field );
    void            setSize( int size );
    void            setAffectedRows( int rows );
private:
    int 	    sz;
    int 	    affRows;
    QSqlFieldList   fieldList;
};

#endif	// QT_NO_SQL
#endif
