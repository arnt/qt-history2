/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qprogbar.h#2 $
**
** Definition of QProgressBar class
**
** Created : 970520
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPROGBAR_H
#define QPROGBAR_H

#include "qdialog.h"
#include "qdatetm.h"
#include "qpushbt.h"
#include "qlabel.h"


class QProgressBar : public QDialog
{
    Q_OBJECT
public:
    QProgressBar( const char* label, int totalsteps, QWidget *parent=0,
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
    void	paintEvent( QPaintEvent * );
    virtual bool setIndicator( QString& progress_str, int progress,
        int totalsteps );

private:
    int		totalsteps;
    int		progress;
    int		percentage;
    QString	progress_str;
    QTime	starttime;
    QPushButton	cancel;
    bool	cancellation_flag;
    QLabel	label;
    QRect	barArea() const;
    QCursor	parentCursor;

private:	// Disabled copy constructor and operator=
    QProgressBar( const QProgressBar & ) {}
    QProgressBar &operator=( const QProgressBar & ) { return *this; }
};

inline int QProgressBar::totalSteps() const { return totalsteps; }

#endif // QPROGBAR_H
