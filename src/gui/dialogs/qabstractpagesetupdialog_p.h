/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QABSTRACTPAGESETUPDIALOG_P_H
#define QABSTRACTPAGESETUPDIALOG_P_H

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

#ifndef QT_NO_PRINTDIALOG

#include "private/qdialog_p.h"

#include "qpagesetupdialog.h"

QT_BEGIN_NAMESPACE

class QPrinter;

class QAbstractPageSetupDialogPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(QAbstractPageSetupDialog)

public:
    QAbstractPageSetupDialogPrivate() : printer(0), pageSetupDialogOptions(0) {}
    void addEnabledOption(QPageSetupDialog::PageSetupDialogOption option) { pageSetupDialogOptions |= option; }
    void setEnabledOptions(QPageSetupDialog::PageSetupDialogOptions options) { pageSetupDialogOptions = options; }
    QPageSetupDialog::PageSetupDialogOptions enabledOptions() const { return pageSetupDialogOptions; }
    bool isOptionEnabled(QPageSetupDialog::PageSetupDialogOption option) const { return pageSetupDialogOptions & option; }

    QPrinter *printer;
    QPageSetupDialog::PageSetupDialogOptions pageSetupDialogOptions;
};

#endif // QT_NO_PRINTDIALOG

QT_END_NAMESPACE

#endif // QABSTRACTPAGESETUPDIALOG_P_H
