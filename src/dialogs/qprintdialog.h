/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qprintdialog.h#17 $
**
** Definition of print dialog.
**
** Created : 950829
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPRNDLG_H
#define QPRNDLG_H

#ifndef QT_H
#include "qdialog.h"
#endif // QT_H

class QGroupBox;


struct QPrintDialogPrivate;


class QPrintDialog : public QDialog
{
    Q_OBJECT
public:
    QPrintDialog( QPrinter *, QWidget *parent=0, const char *name=0 );
    ~QPrintDialog();

    static bool getPrinterSetup( QPrinter * );

    void setPrinter( QPrinter *, bool = FALSE );
    QPrinter * printer() const;

private slots:
    void browseClicked();
    void okClicked();

    void printerOrFileSelected( int );
    void landscapeSelected( int );
    void paperSizeSelected( int );
    void orientSelected( int );
    void pageOrderSelected( int );
    void colorModeSelected( int );
    void setNumCopies( int );
    void printRangeSelected( int );
    void setFirstPage( int );
    void setLastPage( int );

private:
    QPrintDialogPrivate *d;

    QGroupBox * setupDestination();
    QGroupBox * setupOptions();
    QGroupBox * setupPaper();

private:	// Disabled copy constructor and operator=
    QPrintDialog( const QPrintDialog & );
    QPrintDialog &operator=( const QPrintDialog & );
};


#endif // QPRNDLG_H
