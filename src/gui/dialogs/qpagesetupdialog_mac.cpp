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

/*****************************************************************************
  External functions
 *****************************************************************************/
extern void qt_enter_modal(QWidget *); //qapplication_mac.cpp
extern void qt_leave_modal(QWidget *); //qapplication_mac.cpp

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
    { //simulate modality
	QWidget modal_widg(0,
			   Qt::WType_TopLevel | Qt::WStyle_Customize | Qt::WStyle_DialogBorder);
        modal_widg.setObjectName(QLatin1String(__FILE__ "__modal_dlg"));
	qt_enter_modal(&modal_widg);
        if (PMSessionPageSetupDialog(ep->session, ep->format, &ret) != noErr)
            ret = false;
	qt_leave_modal(&modal_widg);
    }
    return ret ? Accepted : Rejected;
}
