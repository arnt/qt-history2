/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSQL_H
#define QSQL_H

#include "qglobal.h"

namespace QSql
{
    enum Location {
        BeforeFirstRecord = -1,
        AfterLastRecord = -2
//#ifdef QT_COMPAT ### should uncomment the check at some point
        , BeforeFirst = BeforeFirstRecord,
        AfterLast = AfterLastRecord
//#endif
    };

    enum ParamTypeFlag {
        In = 0x00000001,
        Out = 0x00000002,
        InOut = In | Out,
        Binary = 0x00000004
    };
    Q_DECLARE_FLAGS(ParamType, ParamTypeFlag)

    enum TableType {
        Tables = 0x01,
        SystemTables = 0x02,
        Views = 0x04,
        AllTables = 0xff
    };

#ifdef QT_COMPAT
    enum Op {
        None = -1,
        Insert = 0,
        Update = 1,
        Delete = 2
    };

    enum Confirm {
        Cancel = -1,
        No = 0,
        Yes = 1
    };
#endif
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QSql::ParamType)

#endif
