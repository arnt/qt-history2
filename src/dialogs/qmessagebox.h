/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qmessagebox.h#42 $
**
** Definition of QMessageBox class
**
** Created : 950503
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QMESSAGEBOX_H
#define QMESSAGEBOX_H

#ifndef QT_H
#include "qdialog.h"
#endif // QT_H

class  QLabel;
class  QPushButton;
struct QMBData;


class Q_EXPORT QMessageBox : public QDialog
{
    Q_OBJECT
public:
    enum Icon { NoIcon = 0, Information = 1, Warning = 2, Critical = 3 };

    QMessageBox( QWidget *parent=0, const char *name=0 );
    QMessageBox( const QString& caption, const QString &text, Icon icon,
		 int button0, int button1, int button2,
		 QWidget *parent=0, const char *name=0, bool modal=TRUE,
		 WFlags f=WStyle_DialogBorder  );
   ~QMessageBox();

    enum { Ok = 1, Cancel = 2, Yes = 3, No = 4, Abort = 5, Retry = 6,
	   Ignore = 7, ButtonMask = 0x07,
	   Default = 0x100, Escape = 0x200, FlagMask = 0x300 };

    static int information( QWidget *parent, const QString &caption,
			    const QString& text,
			    int button0, int button1=0, int button2=0 );
    static int information( QWidget *parent, const QString &caption,
			    const QString& text,
			    const QString& button0Text = "OK",
			    const QString& button1Text = QString::null,
			    const QString& button2Text = QString::null,
			    int defaultButtonNumber = 0,
			    int escapeButtonNumber = -1 );

    static int warning( QWidget *parent, const QString &caption,
			const QString& text,
			int button0, int button1, int button2=0 );
    static int warning( QWidget *parent, const QString &caption,
			const QString& text,
			const QString& button0Text = "OK",
			const QString& button1Text = QString::null,
			const QString& button2Text = QString::null,
			int defaultButtonNumber = 0,
			int escapeButtonNumber = -1 );

    static int critical( QWidget *parent, const QString &caption,
			 const QString& text,
			 int button0, int button1, int button2=0 );
    static int critical( QWidget *parent, const QString &caption,
			 const QString& text,
			 const QString& button0Text = "OK",
			 const QString& button1Text = QString::null,
			 const QString& button2Text = QString::null,
			 int defaultButtonNumber = 0,
			 int escapeButtonNumber = -1 );

    static void about( QWidget *parent, const QString &caption,
		       const QString& text );

    static void aboutQt( QWidget *parent, const QString& caption=QString::null );

#if 1 /* OBSOLETE */
    static int message( const QString &caption,
			const QString& text,  const QString& buttonText=QString::null,
			QWidget *parent=0, const char *name=0 );

    static bool query( const QString &caption,
		       const QString& text,  const QString& yesButtonText=QString::null,
		       const QString& noButtonText=QString::null,
		       QWidget *parent=0, const char *name=0 );
#endif

    QString	text() const;
    void	setText( const QString &);

    Icon	icon() const;
    void	setIcon( Icon );

    const QPixmap *iconPixmap() const;
    void	setIconPixmap( const QPixmap & );

#if 1 /* OBSOLETE */
    QString	buttonText() const;
    void	setButtonText( const QString &);
#endif

    QString	buttonText( int button ) const;
    void	setButtonText( int button, const QString &);

    void	adjustSize();

    void	setStyle( GUIStyle );

    static QPixmap standardIcon( Icon icon, GUIStyle style );

protected:
    void	resizeEvent( QResizeEvent * );
    void	keyPressEvent( QKeyEvent * );

private slots:
    void	buttonClicked();

private:
    void	init( int, int, int );
    int		indexOf( int ) const;
    void	resizeButtons();
    QLabel     *label;
    QMBData    *mbd;
    void       *reserved1;
    void       *reserved2;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QMessageBox( const QMessageBox & );
    QMessageBox &operator=( const QMessageBox & );
#endif
};


#endif // QMESSAGEBOX_H
