/****************************************************************************
 **
 ** Definition of some Qt private functions.
 **
 ** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
 **
 ** This file is part of the kernel module of the Qt GUI Toolkit.
 ** EDITIONS: FREE, ENTERPRISE
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

#ifndef QSQLMODEL_P_H
#define QSQLMODEL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qsql*model.h .  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//
//

#include "private/qabstractitemmodel_p.h"
#include "qlist.h"
#include "qsqlquery.h"
#include "qsqlrecord.h"
#include "qvarlengtharray.h"
#include "qvector.h"

class QSqlModelPrivate: public QAbstractItemModelPrivate
{
    Q_DECLARE_PUBLIC(QSqlModel);
public:
    QSqlModelPrivate(): lastRowIndex(-1), atEnd(false) {}

    void prefetch();

    mutable QSqlQuery query;
    mutable QSqlError error;
    QModelIndex bottom;
    QSqlRecord rec;
    int lastRowIndex;
    uint atEnd : 1;
    QVector<QVariant> headers;
    QVarLengthArray<int, 56> colOffsets; // used to calculate dataIndex of columns
};

#endif

