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

#include "qdialog_p.h"

class QPrinter;

class QAbstractPageSetupDialogPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(QAbstractPageSetupDialog)

public:
    QPrinter *printer;
};

#endif // QABSTRACTPAGESETUPDIALOG_P_H
