/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qmessagebox.h#17 $
**
** Definition of QMessageBox class
**
** Created : 950503
**
** Copyright (C) 1992-1996 Troll Tech AS.  All rights reserved.
**
** This file is part of the non-commercial distribution of Qt 1.1.
**
** See the file LICENSE included in the distribution for the usage
** and distribution terms, or http://www.troll.no/qt/license.html.
**
** (This file differs from the one in the commercial distribution of
** Qt only by this comment.)
**
*****************************************************************************/

#ifndef QMSGBOX_H
#define QMSGBOX_H

#include "qdialog.h"

class QLabel;
class QPushButton;


class QMessageBox : public QDialog
{
    Q_OBJECT
public:
    static int message( const char *caption,
			const char *text,  const char *buttonText=0,
			QWidget *parent=0, const char *name=0 );

    static bool query( const char *caption,
		       const char *text,  const char *yesButtonText=0,
		       const char *noButtonText=0,
		       QWidget *parent=0, const char *name=0 );

    QMessageBox( QWidget *parent=0, const char *name=0 );

    const char *text()		const;
    void	setText( const char * );

    const char *buttonText()	const;
    void	setButtonText( const char * );

    void	adjustSize();

protected:
    void	resizeEvent( QResizeEvent * );

private:
    QLabel	*label;
    QPushButton *button;
    void	*reserved1;
    void	*reserved2;

    QPushButton *button2() { return (QPushButton*) reserved1; }

private:	// Disabled copy constructor and operator=
    QMessageBox( const QMessageBox & ) {}
    QMessageBox &operator=( const QMessageBox & ) { return *this; }
};


#endif // QMSGBOX_H
