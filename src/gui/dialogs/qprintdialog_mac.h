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

#ifndef QPRINTDIALOG_MAC_H
#define QPRINTDIALOG_MAC_H

#include <qabstractprintdialog.h>

class QPrintDialogMacPrivate;
class QPrinter;

class Q_GUI_EXPORT QPrintDialogMac : public QAbstractPrintDialog
{
    Q_DECLARE_PRIVATE(QPrintDialogMac)
public:
    QPrintDialogMac(QPrinter *printer, QWidget *parent = 0);
    
    int exec();

private:
#if defined(Q_DISABLE_COPY)
    QPrintDialogMac(const QPrintDialogMac &);
    QPrintDialogMac &operator=(const QPrintDialogMac &);
#endif
};

#endif // QPRINTDIALOG_MAC_H
