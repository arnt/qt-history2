/****************************************************************************
**
** Definition of QSqlDriverInterface class.
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

#ifndef QSQLDRIVERINTERFACE_H
#define QSQLDRIVERINTERFACE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//
//

#ifndef QT_H
#include <private/qcom_p.h>
#endif // QT_H

#if !defined( QT_MODULE_SQL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_EXPORT_SQL
#else
#define QM_EXPORT_SQL Q_SQL_EXPORT
#endif

#ifndef QT_NO_SQL

#ifndef QT_NO_COMPONENT

// {EDDD5AD5-DF3C-400c-A711-163B72FE5F61}
#ifndef IID_QSqlDriverFactory
#define IID_QSqlDriverFactory QUuid(0xeddd5ad5, 0xdf3c, 0x400c, 0xa7, 0x11, 0x16, 0x3b, 0x72, 0xfe, 0x5f, 0x61)
#endif

class QSqlDriver;

struct QM_EXPORT_SQL QSqlDriverFactoryInterface : public QFeatureListInterface
{
    virtual QSqlDriver* create( const QString& name ) = 0;
};

#endif //QT_NO_COMPONENT
#endif // QT_NO_SQL

#endif // QSQLDRIVERINTERFACE_P_H
