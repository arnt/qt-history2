/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qprogressdialog.h#8 $
**
** Definition of QProgressDialog class
**
** Created : 970520
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPROGDLG_H
#define QPROGDLG_H

#include "qsemimodal.h"
#include "qpushbt.h"
#include "qlabel.h"
#include "qprogbar.h"

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
    void	   center();
    void	   layout();
    QLabel	  *label()  const;
    QPushButton	  *cancel() const;
    QProgressBar  *bar()    const;
    QProgressData *d;

private:	// Disabled copy constructor and operator=
    QProgressDialog( const QProgressDialog & ) {}
    QProgressDialog &operator=( const QProgressDialog & ) { return *this; }
};


#endif // QPROGDLG_H
