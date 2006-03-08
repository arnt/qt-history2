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

#include <QtGui/qabstractprintdialog.h>

QT_MODULE(Gui)

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
    Q_PRIVATE_SLOT(d_func(), void _q_browseClicked())
    Q_PRIVATE_SLOT(d_func(), void _q_okClicked())
    Q_PRIVATE_SLOT(d_func(), void _q_printerOrFileSelected(QAbstractButton *))
    Q_PRIVATE_SLOT(d_func(), void _q_landscapeSelected(int))
    Q_PRIVATE_SLOT(d_func(), void _q_paperSizeSelected(int))
    Q_PRIVATE_SLOT(d_func(), void _q_orientSelected(int))
    Q_PRIVATE_SLOT(d_func(), void _q_pageOrderSelected(QAbstractButton *))
    Q_PRIVATE_SLOT(d_func(), void _q_colorModeSelected(QAbstractButton *))
    Q_PRIVATE_SLOT(d_func(), void _q_setNumCopies(int))
    Q_PRIVATE_SLOT(d_func(), void _q_printRangeSelected(QAbstractButton *))
    Q_PRIVATE_SLOT(d_func(), void _q_setFirstPage(int))
    Q_PRIVATE_SLOT(d_func(), void _q_setLastPage(int))
    Q_PRIVATE_SLOT(d_func(), void _q_fileNameEditChanged(const QString &text))
// #endif
};

#endif // QT_NO_PRINTDIALOG

#endif // QPRINTDIALOG_H
