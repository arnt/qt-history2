/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qmessagebox.h#9 $
**
** Definition of QMessageBox class
**
** Created : 950503
**
** Copyright (C) 1995-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QMSGBOX_H

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

private:	// Disabled copy constructor and operator=
    QMessageBox( const QMessageBox & ) {}
    QMessageBox &operator=( const QMessageBox & ) { return *this; }
};


#endif // QMSGBOX_H
