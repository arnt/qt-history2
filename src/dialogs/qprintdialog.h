/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qprintdialog.h#23 $
**
** Definition of print dialog.
**
** Created : 950829
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QPRINTDIALOG_H
#define QPRINTDIALOG_H

#ifndef QT_H
#include "qdialog.h"
#endif // QT_H

#ifndef QT_NO_PRINTDIALOG

class QGroupBox;


struct QPrintDialogPrivate;


class Q_EXPORT QPrintDialog : public QDialog
{
    Q_OBJECT
public:
    QPrintDialog( QPrinter *, QWidget *parent=0, const char *name=0 );
    ~QPrintDialog();

    static bool getPrinterSetup( QPrinter * );

    void setPrinter( QPrinter *, bool = FALSE );
    QPrinter * printer() const;

    void addButton( QPushButton *but );

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

    void fileNameEditChanged( const QString &text );
    
private:
    QPrintDialogPrivate *d;

    QGroupBox * setupDestination();
    QGroupBox * setupOptions();
    QGroupBox * setupPaper();
    QGroupBox * setupPrinterSettings();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPrintDialog( const QPrintDialog & );
    QPrintDialog &operator=( const QPrintDialog & );
#endif
};

#endif

#endif // QPRINTDIALOG_H
