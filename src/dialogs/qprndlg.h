/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qprndlg.h#9 $
**
**		      ***   INTERNAL HEADER FILE   ***
**
**		This file is NOT a part of the Qt interface!
**
** Definition of internal print dialog (X11) used by QPrinter::select().
**
** Created : 950829
**
** Copyright (C) 1995-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPRNDLG_H
#define QPRNDLG_H

#include "qdialog.h"


class QPrintDialog : public QDialog
{
    Q_OBJECT
public:
    QPrintDialog( QPrinter *, QWidget *parent=0, const char *name=0 );

private slots:
    void	printerOrFileSelected( int );
    void	browseClicked();
    void	okClicked();

private:
    QPrinter *printer;

private:	// Disabled copy constructor and operator=
    QPrintDialog( const QPrintDialog & ) {}
    QPrintDialog &operator=( const QPrintDialog & ) { return *this; }
};


#endif // QPRNDLG_H
