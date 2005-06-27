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

#ifndef QPRINTDIALOG_H
#define QPRINTDIALOG_H

#include "QtGui/qabstractprintdialog.h"

#ifndef QT_NO_PRINTDIALOG

class QPrintDialogPrivate;
class QAbstractButton;
class QPrinter;

class Q_GUI_EXPORT QPrintDialog : public QAbstractPrintDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QPrintDialog)
public:
    explicit QPrintDialog(QPrinter *printer, QWidget *parent = 0);
    ~QPrintDialog();

#if defined (Q_OS_UNIX) && !defined (Q_OS_MAC) && defined (QT3_SUPPORT)
    void setPrinter(QPrinter *, bool = false);
    QPrinter *printer() const;
    void addButton(QPushButton *button);
#endif

    int exec();

private:
    Q_DISABLE_COPY(QPrintDialog)

// #if defined (Q_OS_UNIX) && !defined (Q_OS_MAC)
    Q_PRIVATE_SLOT(d_func(), void browseClicked())
    Q_PRIVATE_SLOT(d_func(), void okClicked())
    Q_PRIVATE_SLOT(d_func(), void printerOrFileSelected(QAbstractButton *))
    Q_PRIVATE_SLOT(d_func(), void landscapeSelected(int))
    Q_PRIVATE_SLOT(d_func(), void paperSizeSelected(int))
    Q_PRIVATE_SLOT(d_func(), void orientSelected(int))
    Q_PRIVATE_SLOT(d_func(), void pageOrderSelected(QAbstractButton *))
    Q_PRIVATE_SLOT(d_func(), void colorModeSelected(QAbstractButton *))
    Q_PRIVATE_SLOT(d_func(), void setNumCopies(int))
    Q_PRIVATE_SLOT(d_func(), void printRangeSelected(QAbstractButton *))
    Q_PRIVATE_SLOT(d_func(), void setFirstPage(int))
    Q_PRIVATE_SLOT(d_func(), void setLastPage(int))
    Q_PRIVATE_SLOT(d_func(), void fileNameEditChanged(const QString &text))
// #endif
};

#endif // QT_NO_PRINTDIALOG
#endif // QPRINTDIALOG_H
