/****************************************************************************
**
** Definition of print dialog.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPRINTDIALOG_H
#define QPRINTDIALOG_H

#ifndef QT_H
#include "qdialog.h"
#endif // QT_H

#ifndef QT_NO_PRINTDIALOG

class QGroupBox;
class QPrintDialogPrivate;
class QListView;

class Q_GUI_EXPORT QPrintDialog : public QDialog
{
    Q_OBJECT
public:
    QPrintDialog(QPrinter *, QWidget* parent=0, const char* name=0);
    ~QPrintDialog();

    static bool getPrinterSetup(QPrinter *, QWidget* = 0);
    static void setGlobalPrintDialog(QPrintDialog *);

    void setPrinter(QPrinter *, bool = false);
    QPrinter * printer() const;

    void addButton(QButton *but);

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
    QPrintDialogPrivate *d;

    QGroupBox * setupDestination();
    QGroupBox * setupOptions();
    QGroupBox * setupPaper();
    QGroupBox * setupPrinterSettings();

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPrintDialog(const QPrintDialog &);
    QPrintDialog &operator=(const QPrintDialog &);
#endif
};

#endif

#endif // QPRINTDIALOG_H
