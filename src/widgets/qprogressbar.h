/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qprogressbar.h#8 $
**
** Definition of QProgressBar class
**
** Created : 970520
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPROGBAR_H
#define QPROGBAR_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H


class QProgressBar : public QFrame
{
    Q_OBJECT
public:
    QProgressBar( QWidget *parent=0, const char *name=0, WFlags f=0 );
    QProgressBar( int totalSteps, QWidget *parent=0, const char *name=0,
		  WFlags f=0 );

    int		totalSteps() const;
    int		progress()   const;

    QSize	sizeHint() const;
    void	show();

public slots:
    void	reset();
    void	setTotalSteps( int totalSteps );
    void	setProgress( int progress );

protected:
    void	drawContents( QPainter * );
    virtual bool setIndicator( QString& progress_str, int progress,
			       int totalSteps );

private:
    int		total_steps;
    int		progress_val;
    int		percentage;
    QString	progress_str;

private:	// Disabled copy constructor and operator=
    QProgressBar( const QProgressBar & );
    QProgressBar &operator=( const QProgressBar & );
};


inline int QProgressBar::totalSteps() const
{
    return total_steps;
}

inline int QProgressBar::progress() const
{
    return progress_val;
}


#endif // QPROGBAR_H
