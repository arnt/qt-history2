/****************************************************************************
**
** Definition of QSql class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSQL_H
#define QSQL_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H

#if !defined( QT_MODULE_SQL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_EXPORT_SQL
#else
#define QM_EXPORT_SQL Q_SQL_EXPORT
#endif

#ifndef QT_NO_SQL

class QM_EXPORT_SQL QSql
{
public:
    QSql() {}
    enum Op {
	None = -1,
	Insert = 0,
	Update = 1,
	Delete = 2
    };

    enum Location {
	BeforeFirst = -1,
	AfterLast = -2
    };

    enum Confirm {
	Cancel = -1,
	No = 0,
	Yes = 1
    };

    enum ParamTypeFlags {
	In = 0x00000001,
	Out = 0x00000002,
	InOut = 0x00000003, // In | Out
	Binary = 0x00000004
    };
    Q_DECLARE_FLAGS(ParamType, ParamTypeFlags);

    enum TableType {
	Tables = 0x01,
	SystemTables = 0x02,
	Views = 0x04,
	AllTables = 0xff
    };

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSql( const QSql & );
    QSql &operator=( const QSql & );
#endif

};

Q_DECLARE_OPERATORS_FOR_FLAGS(QSql::ParamType)

#endif
#endif
