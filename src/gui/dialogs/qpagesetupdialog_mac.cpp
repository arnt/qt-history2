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


#include "qpagesetupdialog.h"

#include <private/qprintengine_mac_p.h>
#include <private/qabstractpagesetupdialog_p.h>

#define d d_func()
#define q q_func()

class QPageSetupDialogPrivate : public QAbstractPageSetupDialogPrivate
{
};

QPageSetupDialog::QPageSetupDialog(QPrinter *printer, QWidget *parent)
    : QAbstractPageSetupDialog(*(new QPageSetupDialogPrivate), printer, parent)
{
}

int QPageSetupDialog::exec()
{
    QMacPrintEngine *engine = static_cast<QMacPrintEngine *>(d->printer->paintEngine());
    QMacPrintEnginePrivate *ep = static_cast<QMacPrintEnginePrivate *>(engine->d_ptr);
    Boolean ret;
    if (PMSessionPageSetupDialog(ep->session, ep->format, &ret) != noErr)
        return Rejected;
    return ret == true ? Accepted : Rejected;
}
