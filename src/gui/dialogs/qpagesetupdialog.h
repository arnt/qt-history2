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

#ifndef QPAGESETUPDIALOG_H
#define QPAGESETUPDIALOG_H

#include "QtGui/qabstractpagesetupdialog.h"

#ifndef QT_NO_PRINTDIALOG

class QPageSetupDialogPrivate;

class Q_GUI_EXPORT QPageSetupDialog : public QAbstractPageSetupDialog
{
    Q_DECLARE_PRIVATE(QPageSetupDialog)
public:
    explicit QPageSetupDialog(QPrinter *printer, QWidget *parent = 0);

    virtual int exec();
};

#endif // QT_NO_PRINTDIALOG
#endif // QPAGESETUPDIALOG_H
