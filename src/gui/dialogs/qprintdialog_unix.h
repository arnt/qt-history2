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

#ifndef QPRINTDIALOG_UNIX_H
#define QPRINTDIALOG_UNIX_H

#ifndef QT_H
#include "qabstractprintdialog.h"
#endif // QT_H

#ifndef QT_NO_PRINTDIALOG

class QGroupBox;
class QPrintDialogUnixPrivate;
class QListView;

class Q_GUI_EXPORT QPrintDialogUnix : public QAbstractPrintDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QPrintDialogUnix)
public:
    QPrintDialogUnix(QPrinter *, QWidget* parent=0);
    ~QPrintDialogUnix();

    int exec();

    void setPrinter(QPrinter *, bool = false);
    QPrinter * printer() const;

    void addButton(QPushButton *but);

private slots:
    void browseClicked();
    void okClicked();

    void printerOrFileSelected(int);
    void landscapeSelected(int);
    void paperSizeSelected(int);
    void orientSelected(int);
    void pageOrderSelected(int);
    void colorModeSelected(int);
    void setNumCopies(int);
    void printRangeSelected(int);
    void setFirstPage(int);
    void setLastPage(int);

    void fileNameEditChanged(const QString &text);

private:
    QGroupBox * setupDestination();
    QGroupBox * setupOptions();
    QGroupBox * setupPaper();
    QGroupBox * setupPrinterSettings();

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPrintDialogUnix(const QPrintDialogUnix &);
    QPrintDialogUnix &operator=(const QPrintDialogUnix &);
#endif
};

#endif

#endif // QPRINTDIALOG_UNIX_H
