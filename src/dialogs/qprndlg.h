/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qprndlg.h#2 $
**
**		      ***   INTERNAL HEADER FILE   ***
**
**		This file is NOT a part of the Qt interface!
**
** Internal print dialog for the QPrinter class.
**
** Author  : Haavard Nord
** Created : 950829
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPRNDLG_H
#define QPRNDLG_H

#include <qdialog.h>


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
};


#endif // QPRNDLG_H
