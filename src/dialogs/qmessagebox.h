/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qmessagebox.h#2 $
**
** Definition of QMessageBox class
**
** Author  : Haavard Nord
** Created : 950503
**
** Copyright (C) 1995 by Troll Tech AS.	 All rights reserved.
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
			const char *text,  const char *buttonText,
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
};


#endif // QMSGBOX_H
