/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qprogressdialog.h#14 $
**
** Definition of QProgressDialog class
**
** Created : 970520
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
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


class QProgressDialog : public QSemiModal
{
    Q_OBJECT
public:
    QProgressDialog( QWidget *parent=0, const char *name=0, bool modal=FALSE,
		     WFlags f=0 );
    QProgressDialog( const char *labelText, const char *cancelButtonText,
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

public slots:
    void	cancel();
    void	reset();
    void	setTotalSteps( int totalSteps );
    void	setProgress( int progress );
    void	setLabelText( const char * );
    void	setCancelButtonText( const char * );

signals:
    void	cancelled();

protected:
    void	resizeEvent( QResizeEvent * );
    void	styleChange(GUIStyle);

private:
    void	   init( QWidget *creator, const char* lbl, const char* canc,
		         int totstps);
    void	   center();
    void	   layout();
    QLabel	  *label()  const;
    QProgressBar  *bar()    const;
    QProgressData *d;

private:	// Disabled copy constructor and operator=
    QProgressDialog( const QProgressDialog & );
    QProgressDialog &operator=( const QProgressDialog & );
};


#endif // QPROGRESSDIALOG_H
