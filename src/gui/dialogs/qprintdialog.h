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

#if defined (Q_OS_UNIX) && !defined (Q_OS_MAC) && defined (QT_COMPAT)
    void setPrinter(QPrinter *, bool = false);
    QPrinter *printer() const;
    void addButton(QPushButton *button);
#endif

    int exec();

private:
    Q_DISABLE_COPY(QPrintDialog)

// #if defined (Q_OS_UNIX) && !defined (Q_OS_MAC)
    Q_PRIVATE_SLOT(d, void browseClicked())
    Q_PRIVATE_SLOT(d, void okClicked())
    Q_PRIVATE_SLOT(d, void printerOrFileSelected(QAbstractButton *))
    Q_PRIVATE_SLOT(d, void landscapeSelected(int))
    Q_PRIVATE_SLOT(d, void paperSizeSelected(int))
    Q_PRIVATE_SLOT(d, void orientSelected(int))
    Q_PRIVATE_SLOT(d, void pageOrderSelected(QAbstractButton *))
    Q_PRIVATE_SLOT(d, void colorModeSelected(QAbstractButton *))
    Q_PRIVATE_SLOT(d, void setNumCopies(int))
    Q_PRIVATE_SLOT(d, void printRangeSelected(QAbstractButton *))
    Q_PRIVATE_SLOT(d, void setFirstPage(int))
    Q_PRIVATE_SLOT(d, void setLastPage(int))
    Q_PRIVATE_SLOT(d, void fileNameEditChanged(const QString &text))
// #endif
};

#endif // QPRINTDIALOG_H
