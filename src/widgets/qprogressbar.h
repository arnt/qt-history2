/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qprogressbar.h#18 $
**
** Definition of QProgressBar class
**
** Created : 970520
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

#ifndef QPROGRESSBAR_H
#define QPROGRESSBAR_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H


class QProgressBarPrivate;


class Q_EXPORT QProgressBar : public QFrame
{
    Q_OBJECT
public:
    QProgressBar( QWidget *parent=0, const char *name=0, WFlags f=0 );
    QProgressBar( int totalSteps, QWidget *parent=0, const char *name=0,
		  WFlags f=0 );

    int		totalSteps() const;
    int		progress()   const;

    QSize	sizeHint() const;
    QSizePolicy sizePolicy() const;

    void	show();

public slots:
    void	reset();
    virtual void setTotalSteps( int totalSteps );
    virtual void setProgress( int progress );

protected:
    void	drawContents( QPainter * );
    void	drawContentsMask( QPainter * );
    virtual bool setIndicator( QString & progress_str, int progress,
			       int totalSteps );

private:
    int		total_steps;
    int		progress_val;
    int		percentage;
    QString	progress_str;
    QProgressBarPrivate * d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QProgressBar( const QProgressBar & );
    QProgressBar &operator=( const QProgressBar & );
#endif
};


inline int QProgressBar::totalSteps() const
{
    return total_steps;
}

inline int QProgressBar::progress() const
{
    return progress_val;
}


#endif // QPROGRESSBAR_H
