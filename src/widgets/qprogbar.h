/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qprogbar.h#4 $
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

#include "qframe.h"


class QProgressBar : public QFrame
{
    Q_OBJECT
public:
    QProgressBar( int totalsteps, QWidget *parent=0,
	const char *name=0, WFlags f=0, bool allowLines=TRUE );

    void	reset( int totalsteps );
    void	reset();

    QSize	sizeHint() const;

    int		totalSteps () const;
    int		progress () const;
    void	setProgress( int progress );

    void	show();

protected:
    void	drawContents( QPainter * );
    virtual bool setIndicator( QString& progress_str, int progress,
				int totalsteps );

private:
    int		totalsteps;
    int		progr;
    int		percentage;
    QString	progress_str;

private:	// Disabled copy constructor and operator=
    QProgressBar( const QProgressBar & ) {}
    QProgressBar &operator=( const QProgressBar & ) { return *this; }
};

inline int QProgressBar::totalSteps() const { return totalsteps; }
inline int QProgressBar::progress() const { return progr; }

#endif // QPROGBAR_H
