#ifndef QSQL_P_H
#define QSQL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qsqltable.cpp and qsqlform.cpp. This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include "qsqlview.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlPrivate
{
public:
    QSqlPrivate() : nullTxt("<null>"), haveAllRows(FALSE), s( 0 ) {}
    virtual ~QSqlPrivate() { if ( s ) delete s; }

    enum Mode {
	Sql,
	Rowset,
	View
    };

    void resetMode( Mode m )
    {
	delete s;
	switch( m ) {
	case Sql:
	    s = new QSql();
	    break;
	case Rowset:
	    s = new QSqlRowset();
	    break;
	case View:
	    s = new QSqlView();
	    break;
	}
	mode = m;
    }

    QSql* sql()
    {
	// any mode
	return s;
    }

    QSqlRowset* rowset()
    {
	if ( mode == Rowset || mode == View )
	    return (QSqlRowset*)s;
	return 0;
    }

    QSqlView* view()
    {
	if ( mode == View )
	    return (QSqlView*)s;
	return 0;
    }

    QString      nullTxt;
    typedef      QValueList< uint > ColIndex;
    ColIndex     colIndex;
    bool         haveAllRows;

private:
    QSql* s;
    Mode mode;
};

#endif // QT_NO_SQL
#endif // QSQL_P_H
