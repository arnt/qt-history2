/****************************************************************************
** $Id: $
**
** Definition of print dialog.
**
** Created : 950829
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QPRINTDIALOG_H
#define QPRINTDIALOG_H

#ifndef QT_H
#include "qdialog.h"
#endif // QT_H

#ifndef QT_NO_PRINTDIALOG

class QGroupBox;
class QPrintDialogPrivate;
class QListView;

class Q_EXPORT QPrintDialog : public QDialog
{
    Q_OBJECT
public:
    QPrintDialog( QPrinter *, QWidget *parent=0, const char *name=0 );
    ~QPrintDialog();

    static bool getPrinterSetup( QPrinter * );
    static void setGlobalPrintDialog( QPrintDialog * );
    
    void setPrinter( QPrinter *, bool = FALSE );
    QPrinter * printer() const;

    void addButton( QPushButton *but );

    virtual bool setupPrinters ( QListView *printers );
    
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
