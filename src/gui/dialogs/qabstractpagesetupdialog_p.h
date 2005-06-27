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

#ifndef QABSTRACTPAGESETUPDIALOG_P_H
#define QABSTRACTPAGESETUPDIALOG_P_H

#ifndef QT_NO_PRINTDIALOG

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QAbstractItemModel*.  This header file may change from version
// to version without notice, or even be removed.
//
// We mean it.
//
//

#include "qdialog_p.h"

class QPrinter;

class QAbstractPageSetupDialogPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(QAbstractPageSetupDialog)

public:
    QPrinter *printer;
};

#endif // QT_NO_PRINTDIALOG
#endif // QABSTRACTPAGESETUPDIALOG_P_H
