/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qprogressdialog.h#24 $
**
** Definition of QProgressDialog class
**
** Created : 970520
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
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QPROGRESSDIALOG_H
#define QPROGRESSDIALOG_H

#ifndef QT_H
#include "qsemimodal.h"
#include "qpushbutton.h"
#include "qlabel.h"
#include "qprogressbar.h"
#endif // QT_H

struct QProgressData;


class Q_EXPORT QProgressDialog : public QSemiModal
{
    Q_OBJECT
    Q_PROPERTY( bool, "wasCancelled", wasCancelled, 0 )
    Q_PROPERTY( int, "totalSteps", totalSteps, setTotalSteps )
    Q_PROPERTY( int, "progress", progress, setProgress )
    Q_PROPERTY( bool, "autoReset", autoReset, setAutoReset )
    Q_PROPERTY( bool, "autoClose", autoClose, setAutoClose )
    Q_PROPERTY( QString, "labelText", labelText, setLabelText )
	
public:
    QProgressDialog( QWidget *parent=0, const char *name=0, bool modal=FALSE,
		     WFlags f=0 );
    QProgressDialog( const QString& labelText, const QString &cancelButtonText,
		     int totalSteps, QWidget *parent=0, const char *name=0,
		     bool modal=FALSE, WFlags f=0 );
   ~QProgressDialog();

    void	setLabel( QLabel * );
    void	setCancelButton( QPushButton * );
    void	setBar( QProgressBar * );

    bool	wasCancelled() const;

    int		totalSteps() const;
    int		progress()   const;

    QSize	sizeHint() const;

    QString     labelText() const;
    
    void setAutoReset( bool b );
    bool autoReset() const;
    void setAutoClose( bool b );
    bool autoClose() const;

public slots:
    void	cancel();
    void	reset();
    void	setTotalSteps( int totalSteps );
    void	setProgress( int progress );
    void	setLabelText( const QString &);
    void	setCancelButtonText( const QString &);

    void	setMinimumDuration( int ms );
public:
    int		minimumDuration() const;

signals:
    void	cancelled();

protected:
    void	resizeEvent( QResizeEvent * );
    void	closeEvent( QCloseEvent * );
    void	styleChange( QStyle& );

private:
    void	   init( QWidget *creator, const QString& lbl, const QString &canc,
		         int totstps);
    void	   center();
    void	   layout();
    QLabel	  *label()  const;
    QProgressBar  *bar()    const;
    QProgressData *d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QProgressDialog( const QProgressDialog & );
    QProgressDialog &operator=( const QProgressDialog & );
#endif
};


#endif // QPROGRESSDIALOG_H
