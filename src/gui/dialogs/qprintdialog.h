/****************************************************************************
**
** Declaration of QPrintDialog class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPRINTDIALOG_H
#define QPRINTDIALOG_H

#include <qabstractprintdialog.h>

class QPrintDialogPrivate;
class QPrinter;

class Q_GUI_EXPORT QPrintDialog : public QAbstractPrintDialog
{
    Q_DECLARE_PRIVATE(QPrintDialog)
public:
    QPrintDialog(QPrinter *printer, QWidget *parent = 0);
    int exec();
private:
#if defined(Q_DISABLE_COPY)
    QPrintDialog(const QPrintDialog &);
    QPrintDialog &operator=(const QPrintDialog &);
#endif
};

#endif // QPRINTDIALOG_H
