/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qmsgbox.h#15 $
**
** Definition of QMessageBox class
**
** Created : 950503
**
** Copyright (C) 1995-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QMSGBOX_H
#define QMSGBOX_H

#include "qdialog.h"

class  QLabel;
class  QPushButton;
struct QMBData;


class QMessageBox : public QDialog
{
    Q_OBJECT
public:
    enum { Ok = 0x01, Cancel = 0x02, Yes = 0x04, No = 0x08,
	   Abort = 0x10, Retry = 0x20, Ignore = 0x40 };

    static int message( const char *caption,
			const char *text,  const char *buttonText=0,
			QWidget *parent=0, const char *name=0 );

    static bool query( const char *caption,
		       const char *text,  const char *yesButtonText=0,
		       const char *noButtonText=0,
		       QWidget *parent=0, const char *name=0 );

    static int okCancel( const char *caption,
			 const char *text,
			 QWidget *parent=0, const char *name=0 );

    static int yesNo( const char *caption,
		      const char *text,
		      QWidget *parent=0, const char *name=0 );

    static int yesNoCancel( const char *caption,
			    const char *text,
			    QWidget *parent=0, const char *name=0 );

    QMessageBox( QWidget *parent=0, const char *name=0 );
    QMessageBox( int buttons, const char *text, QWidget *parent=0,
		 const char *name=0 );

    const char *text() const;
    void	setText( const char * );

    const char *buttonText() const;
    void	setButtonText( const char * );

    const char *buttonText( int button ) const;
    void	setButtonText( int button, const char * );

    void	adjustSize();

protected:
    void	resizeEvent( QResizeEvent * );
    void	keyPressEvent( QKeyEvent * );

private slots:
    void	buttonClicked();

private:
    void	init( int, const char * );
    QLabel	*label;
    QMBData     *d;
    void        *reserved1;
    void	*reserved2;

private:	// Disabled copy constructor and operator=
    QMessageBox( const QMessageBox & ) {}
    QMessageBox &operator=( const QMessageBox & ) { return *this; }
};


#endif // QMSGBOX_H
