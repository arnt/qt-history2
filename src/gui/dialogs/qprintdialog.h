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

#ifndef QPRINTDIALOG_H
#define QPRINTDIALOG_H

#include <QtGui/qabstractprintdialog.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_PRINTDIALOG

class QPrintDialogPrivate;
class QPushButton;
class QPrinter;

class Q_GUI_EXPORT QPrintDialog : public QAbstractPrintDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QPrintDialog)
public:
    explicit QPrintDialog(QPrinter *printer, QWidget *parent = 0);
    ~QPrintDialog();

    int exec();

#if defined (Q_OS_UNIX) && !defined (Q_OS_MAC) && defined (QT3_SUPPORT)
    void setPrinter(QPrinter *, bool = false);
    QPrinter *printer() const;
    void addButton(QPushButton *button);
#endif

#ifdef QTOPIA_PRINTDIALOG
    bool eventFilter(QObject *, QEvent *);
#endif

private:
#ifndef QTOPIA_PRINTDIALOG
    Q_PRIVATE_SLOT(d_func(), void _q_printToFileChanged(int))
    Q_PRIVATE_SLOT(d_func(), void _q_rbPrintRangeToggled(bool))
    Q_PRIVATE_SLOT(d_func(), void _q_printerChanged(int))
    Q_PRIVATE_SLOT(d_func(), void _q_chbPrintLastFirstToggled(bool))
#ifndef QT_NO_FILEDIALOG
    Q_PRIVATE_SLOT(d_func(), void _q_btnBrowseClicked())
#endif
    Q_PRIVATE_SLOT(d_func(), void _q_btnPropertiesClicked())
#else // QTOPIA_PRINTDIALOG
    Q_PRIVATE_SLOT(d_func(), void _q_okClicked())
    Q_PRIVATE_SLOT(d_func(),void _q_printerOrFileSelected(QAbstractButton *b))
    Q_PRIVATE_SLOT(d_func(),void _q_paperSizeSelected(int))
    Q_PRIVATE_SLOT(d_func(), void _q_orientSelected(int))
    Q_PRIVATE_SLOT(d_func(), void _q_pageOrderSelected(int))
    Q_PRIVATE_SLOT(d_func(), void _q_colorModeSelected(QAbstractButton *))
    Q_PRIVATE_SLOT(d_func(), void _q_setNumCopies(int))
    Q_PRIVATE_SLOT(d_func(), void _q_printRangeSelected(int))
    Q_PRIVATE_SLOT(d_func(), void _q_setFirstPage(int))
    Q_PRIVATE_SLOT(d_func(), void _q_setLastPage(int))
    Q_PRIVATE_SLOT(d_func(), void _q_fileNameEditChanged(const QString &text))
#endif // QTOPIA_PRINTDIALOG
};

#endif // QT_NO_PRINTDIALOG

QT_END_NAMESPACE

QT_END_HEADER

#endif // QPRINTDIALOG_H
