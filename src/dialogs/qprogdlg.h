/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qprogdlg.h#4 $
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

#include "qdialog.h"
#include "qdatetm.h"
#include "qpushbt.h"
#include "qlabel.h"
#include "qprogbar.h"

struct QProgressData;

class QProgressDialog : public QDialog
{
    Q_OBJECT
public:
    QProgressDialog( const char* label, int totalsteps, QWidget *parent=0,
	const char *name=0, bool modal=FALSE, WFlags f=0 );
    ~QProgressDialog();

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
    virtual QProgressBar*	progressBar(int totalsteps);
    virtual QWidget*		labelWidget(const QString&);
    void	styleChange(GUIStyle);

private:
    void	layout();
    QProgressBar*	the_bar;
    QProgressData*	d;

    int		totalsteps;

private:	// Disabled copy constructor and operator=
    QProgressDialog( const QProgressDialog & ) {}
    QProgressDialog &operator=( const QProgressDialog & ) { return *this; }
    void center();
    const QProgressBar& bar() const;
    QProgressBar& bar();
    const QWidget& label() const;
    QWidget& label();
};

inline int QProgressDialog::totalSteps() const { return totalsteps; }

#endif // QPROGBAR_H
