/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qprogressdialog.h#1 $
**
** Definition of QProgressDialog class
**
** Created : 970520
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPROGDLG_H
#define QPROGDLG_H

#include <qdialog.h>
#include <qdatetm.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qprogbar.h>


class QProgressDialog : public QDialog
{
    Q_OBJECT
public:
    QProgressDialog( const char* label, int totalsteps, QWidget *parent=0,
	const char *name=0, bool modal=TRUE, WFlags f=0 );

    void	setCancelButton( const char* );
    void	reset( int totalsteps );

    QSize	sizeHint() const;

    int		totalSteps () const;
    bool	setProgress( int progress );

public slots:
    void	setLabel( const char* );
    void	reset();

signals:
    void	cancelled();

protected:
    void	resizeEvent( QResizeEvent * );
    virtual QProgressBar* progressBar(int totalsteps);

private:
    QProgressBar*	the_bar;
    QTime	starttime;
    QPushButton	cancel;
    bool	cancellation_flag;
    QLabel	label;
    QCursor	parentCursor;
    int		totalsteps;

private:	// Disabled copy constructor and operator=
    QProgressDialog( const QProgressDialog & ) {}
    QProgressDialog &operator=( const QProgressDialog & ) { return *this; }
    const QProgressBar& bar() const;
    QProgressBar& bar();
};

inline int QProgressDialog::totalSteps() const { return totalsteps; }

#endif // QPROGBAR_H
