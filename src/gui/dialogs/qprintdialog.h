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

QT_BEGIN_HEADER

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

private:
    Q_PRIVATE_SLOT(d_func(), void _q_printToFileChanged(int))
    Q_PRIVATE_SLOT(d_func(), void _q_rbPrintRangeToggled(bool))
    Q_PRIVATE_SLOT(d_func(), void _q_printerChanged(int))
    Q_PRIVATE_SLOT(d_func(), void _q_paperSizeChanged(int))
#ifndef QT_NO_FILEDIALOG
    Q_PRIVATE_SLOT(d_func(), void _q_btnBrowseClicked())
#endif
    Q_PRIVATE_SLOT(d_func(), void _q_btnPropertiesClicked())
};



#endif // QT_NO_PRINTDIALOG

QT_END_HEADER

#endif // QPRINTDIALOG_H
