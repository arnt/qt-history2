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

#ifndef QABSTRACTPAGESETUPDIALOG_H
#define QABSTRACTPAGESETUPDIALOG_H

#include "QtGui/qdialog.h"

QT_MODULE(Gui)

#ifndef QT_NO_PRINTDIALOG

class QAbstractPageSetupDialogPrivate;
class QPrinter;

class Q_GUI_EXPORT QAbstractPageSetupDialog : public QDialog
{
    Q_DECLARE_PRIVATE(QAbstractPageSetupDialog)

public:
    explicit QAbstractPageSetupDialog(QPrinter *printer, QWidget *parent = 0);
    QAbstractPageSetupDialog(QAbstractPageSetupDialogPrivate &ptr,
                             QPrinter *printer, QWidget *parent = 0);

    virtual int exec() = 0;

    QPrinter *printer();
};

#endif // QT_NO_PRINTDIALOG
#endif // QABSTRACTPAGESETUPDIALOG_H
